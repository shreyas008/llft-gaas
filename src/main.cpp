/*
 * libdatachannel client example
 * Copyright (c) 2019-2020 Paul-Louis Ageneau
 * Copyright (c) 2019 Murat Dogan
 * Copyright (c) 2020 Will Munn
 * Copyright (c) 2020 Nico Chatzi
 * Copyright (c) 2020 Lara Mackey
 * Copyright (c) 2020 Erik Cota-Robles
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "nanoarch.h"

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

unordered_map<string, shared_ptr<PeerConnection>> peerConnectionMap;
unordered_map<string, shared_ptr<DataChannel>> dataChannelMap;
weak_ptr<DataChannel> weak_dc;

string localId;
bool echoDataChannelMessages = false;

shared_ptr<PeerConnection> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string from_id,
												string to_id, string localId);
void confirmOnStdout(bool echoed, string id, string type, size_t length);
string randomId(size_t length);

/*
** TODO BLOCK
** How to avoid copy
** Data being buffered?
*/

int main(int argc, char **argv) {
	Cmdline *params = nullptr;
	try {
		params = new Cmdline(argc, argv);
	} catch (const std::range_error&e) {
		std::cout<< e.what() << '\n';
		delete params;
		return -1;
	}

	if (argc < 4)
		die("usage: %s <core> <game> <clientId>", argv[0]);

	rtc::InitLogger(LogLevel::Verbose);

	Configuration config;
	string stunServer = "";
	if (params->noStun()) {
		cout << "No stun server is configured.  Only local hosts and public IP addresses suported." << endl;
	} else {
		if (params->stunServer().substr(0,5).compare("stun:") != 0) {
			stunServer = "stun:";
		}
		stunServer += params->stunServer() + ":" + to_string(params->stunPort());
		cout << "Stun server is " << stunServer << endl;
		config.iceServers.emplace_back(stunServer);
	}

	string localId = randomId(6);
	cout << "The local ID is: " << localId << endl;

	echoDataChannelMessages = params->echoDataChannelMessages();
	cout << "Received data channel messages will be "
	     << (echoDataChannelMessages ? "echoed back to sender" : "printed to stdout") << endl;

	auto ws = make_shared<WebSocket>(WebSocket::Configuration{.disableTlsVerification = true});

	ws->onOpen([ws, localId]() {
		cout << "WebSocket connected, signaling ready" << endl;
		json j;
		j["type"] = "new";
		j["data"]["name"] = "ssk";
		j["data"]["id"] = localId;
		j["data"]["user_agent"] = "linux";
		ws->send(j.dump());
	});

	ws->onClosed([]() { cout << "WebSocket closed" << endl; });

	ws->onError([](const string &error) { cout << "WebSocket failed: " << error << endl; });

	ws->onMessage([&](variant<binary, string> data) {
		if (!holds_alternative<string>(data))
			return;

		json message = json::parse(get<string>(data));
		cout << message.dump(2);

		auto it = message["data"].find("from");
		if (it == message["data"].end())
			return;
		string id = it->get<string>();

		it = message.find("type");
		if (it == message.end())
			return;
		string type = it->get<string>();

		shared_ptr<PeerConnection> pc;
		if (auto jt = peerConnectionMap.find(id); jt != peerConnectionMap.end()) {
			cout << "\nFound PeerConnection in map \n";
			pc = jt->second;
		} else if (type == "offer") {
			cout << "Answering to " + id << endl;
			pc = createPeerConnection(config, ws, id, localId, localId);
		} else {
			return;
		}

		if (type == "offer" || type == "answer") {
			auto sdp = message["data"]["description"]["sdp"].get<string>();
			pc->setRemoteDescription(Description(sdp, type));
		} else if (type == "candidate") {
			auto sdp = message["data"]["candidate"]["candidate"].get<string>();
			auto mid = message["data"]["candidate"]["sdpMid"].get<string>();
			pc->addRemoteCandidate(Candidate(sdp, mid));
		}
	});

	string wsPrefix = "";
	if (params->webSocketServer().substr(0,5).compare("ws://") != 0) {
		wsPrefix = "wss://";
	}
	const string url = wsPrefix + params->webSocketServer() + ":" +
		to_string(params->webSocketPort()) + "/ws";
	cout << "Url is " << url << endl;
	ws->open(url);

	cout << "Waiting for signaling to be connected..." << endl;
	while (!ws->isOpen()) {
		if (ws->isClosed())
			return 1;
		this_thread::sleep_for(100ms);
	}

	string id = params->getClientId();
	cout << "Offering to " + id << endl;
	auto pc = createPeerConnection(config, ws, localId, id, localId);

	// We are the offerer, so create a data channel to initiate the process
	const string label = "gameChannel";
	cout << "Creating DataChannel with label \"" << label << "\"" << endl;
	auto dc = pc->createDataChannel(label);

	weak_dc = make_weak_ptr(dc);
	dc->onOpen([id, localId, params]() {
		cout << "DataChannel from " << id << " open" << endl;

		// Starting up nanoarch
		if (!glfwInit())
			die("Failed to initialize glfw");


		core_load(params->coreName());
		core_load_game(params->gameName());

		int video_buffer_size = 3*nwidth*nheight;

		int chunk_size = 1024;
		int nchunks = int(video_buffer_size/chunk_size)+1;
		binary video_chunk(chunk_size);
		vector<binary> video_data; // The video data to be transmitted.
		for (int i=0; i < nchunks; i++) video_data.push_back(video_chunk);

		GLubyte *data = (GLubyte*)malloc((video_buffer_size)*sizeof(int));

		while (!glfwWindowShouldClose(g_win)) {
			glfwPollEvents();

			g_retro.retro_run();

			glClear(GL_COLOR_BUFFER_BIT);

			video_render();

			// glReadPixels(0, 0, nwidth, nheight, GL_RGB, GL_UNSIGNED_BYTE, data);

			// // Load video into binary
			// for (int i = 0; i< nchunks; i++)
			// {
			// 	for (int j=0; j < chunk_size; j++)
			// 	{
			// 		video_data[i][j] = (byte)data[i*chunk_size + j];
			// 	}
			// 	if (auto dc = weak_dc.lock()) dc->send(video_data[i]);
			// }

			if (auto dc = weak_dc.lock()) dc->send("hello");

			glfwSwapBuffers(g_win);
		}

		free(data);
		core_unload();
		audio_deinit();
		video_deinit();

		glfwTerminate();
	});

	dc->onClosed([id]() { cout << "DataChannel from " << id << " closed" << endl; });

	dc->onMessage([id, wdc = make_weak_ptr(dc)](const variant<binary, string> &message) { //handle user input here and exit as well.
		static bool firstMessage = true;
		if (holds_alternative<string>(message) && (!echoDataChannelMessages || firstMessage)) {
			cout << "Message from " << id << " received: " << get<string>(message) << endl;
			firstMessage = false;
		} else if (echoDataChannelMessages) {
			bool echoed = false;
			if (auto dc = wdc.lock()) {
				dc->send(message);
				echoed = true;
			}
			confirmOnStdout(echoed, id, (holds_alternative<string>(message) ? "text" : "binary"),
					get<string>(message).length());
		}
	});

	dataChannelMap.emplace(id, dc);

	while(1) {};

	cout << "Cleaning up..." << endl;

	dataChannelMap.clear();
	peerConnectionMap.clear();
	delete params;
	return 0;
}

// Create and setup a PeerConnection
shared_ptr<PeerConnection> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string from_id,
												string to_id, string localId) {
	auto pc = make_shared<PeerConnection>(config);

	pc->onStateChange([](PeerConnection::State state) { cout << "State: " << state << endl; });

	pc->onGatheringStateChange(
	    [](PeerConnection::GatheringState state) { cout << "Gathering State: " << state << endl; });

	pc->onLocalDescription([wws, from_id, to_id](Description description) {
		json message;
		message["type"] = description.typeString();
		message["data"]["to"] = to_id;
		message["data"]["from"] = from_id;
		message["data"]["session_id"] = from_id + "-" + to_id;
		message["data"]["media"] = "data";
		message["data"]["description"]["sdp"] = string(description);
		message["data"]["description"]["type"] = "offer";
		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onLocalCandidate([wws, from_id, to_id](Candidate candidate) {
		json message;
		message["type"] = "candidate";
		message["data"]["to"] = to_id;
		message["data"]["from"] = from_id;
		message["data"]["session_id"] = from_id + "-" + to_id;
		message["data"]["candidate"]["candidate"] = string(candidate);//.substr(2,string::npos);
		message["data"]["candidate"]["sdpMLineIndex"] = 0;
		message["data"]["candidate"]["sdpMid"] = candidate.mid();

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onDataChannel([from_id, to_id, localId](shared_ptr<DataChannel> dc) {
		cout << "DataChannel from " << from_id << " received with label \"" << dc->label() << "\""
		     << endl;

		dc->onClosed([from_id]() { cout << "DataChannel from " << from_id << " closed" << endl; });

		dc->onMessage([from_id, wdc = make_weak_ptr(dc)](const variant<binary, string> &message) {
			static bool firstMessage = true;
			if (holds_alternative<string>(message) && (!echoDataChannelMessages || firstMessage)) {
				cout << "Message from " << from_id << " received: " << get<string>(message) << endl;
				firstMessage = false;
			} else if (echoDataChannelMessages) {
				bool echoed = false;
				if (auto dc = wdc.lock()) {
					dc->send(message);
					echoed = true;
				}
				confirmOnStdout(echoed, from_id, (holds_alternative<string>(message) ? "text" : "binary"),
						get<string>(message).length());
			}
		});

		dc->send("Hello from " + localId);

		dataChannelMap.emplace(from_id, dc);
	});

	peerConnectionMap.emplace(to_id, pc);
	return pc;
};

void confirmOnStdout(bool echoed, string id, string type, size_t length) {
	static long count = 0;
	static long freq = 100;
	if (!(++count%freq)) {
		cout << "Received " << count << " pings in total from host " << id << ", most recent of type "
		     << type << " and " << (echoed ? "" : "un") << "successfully echoed most recent ping of size "
		     << length << " back to " << id << endl;
		if (count >= (freq * 10) && freq < 1000000) {
			freq *= 10;
		}
	}
}

// Helper function to generate a random ID
string randomId(size_t length) {
	// static const string characters(
	//     "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	static const string characters("0123456789");
	string id(length, '0');
	default_random_engine rng(random_device{}());
	uniform_int_distribution<int> dist(0, int(characters.size() - 1));
	generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
	return id;
}
