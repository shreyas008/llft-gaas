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

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

unordered_map<string, shared_ptr<Client>> clients{};
weak_ptr<DataChannel> weak_dc;

string localId;
xdo_t* xwin = xdo_new(NULL); // handle to window
Window nano_win = 0;

shared_ptr<Client> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string from_id,
												string to_id, string localId);

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

	rtc::InitLogger(LogLevel::Debug);

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

		shared_ptr<Client> c;
		shared_ptr<PeerConnection> pc;
		if (auto jt = clients.find(id); jt != clients.end()) {
			cout << "\nFound PeerConnection in map \n";
			pc = clients.at(id)->peerConnection;
		} else if (type == "offer") {
			cout << "Answering to " + id << endl;
			c = createPeerConnection(config, ws, id, localId, localId);
			clients.emplace(id, c);
		} else {
			return;
		}

		if (type == "offer" || type == "answer") {
			auto sdp = message["data"]["description"]["sdp"].get<string>();
			pc = clients.at(id)->peerConnection;
			pc->setRemoteDescription(Description(sdp, type));
		} else if (type == "candidate") {
			auto sdp = message["data"]["candidate"]["candidate"].get<string>();
			auto mid = message["data"]["candidate"]["sdpMid"].get<string>();
			pc = clients.at(id)->peerConnection;
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
	auto c = createPeerConnection(config, ws, localId, id, localId);
	auto dc = c->dataChannel;

	weak_dc = make_weak_ptr(dc);
	bool channelIsOpen = false;
	dc->onOpen([id, &channelIsOpen]() {
		cout << "DataChannel from " << id << " open" << endl;
		channelIsOpen = true;
	});

	dc->onClosed([id]() { cout << "DataChannel from " << id << " closed" << endl; });

	dc->onMessage([id, wdc = make_weak_ptr(dc)](const variant<binary, string> &message) { //handle user input here and exit as well.
		if (nano_win == 0)  nano_win = get_window(xwin);

		string msg = get<string>(message);
		cout << "Message is: " << msg << endl;
		xdo_send_keysequence_window(xwin, nano_win, msg.c_str(), 0);
	});

	while(!channelIsOpen) {}; //busy wait for channel to open.

	// Start collab-render as a separate process
	if (fork() == 0)
	{
		char* args[] = { "../bin/collab-render",(char*)params->coreName(), (char*)params->gameName(), NULL};
		int err = execvp(args[0], args);
	}

	// Starting up nanoarch
	if (!glfwInit())
		die("Failed to initialize glfw");

	core_load(params->coreName());
	core_load_game(params->gameName());

	nwidth = nwidth/3; nheight = nheight/3;  //resizing to reduce amount of data sent
	glfwSetWindowSize(g_win, nwidth, nheight);

	unsigned long int video_buffer_size = 3*nwidth*nheight;
	unsigned long int compressed_size = video_buffer_size;

	if (auto dc = weak_dc.lock()) //transmit resolution
	{
		json j; j["width"] = nwidth; j["height"] = nheight;
		dc->send(j.dump());
	}

	GLubyte *data = (GLubyte*)malloc((video_buffer_size)*sizeof(GLubyte));
	GLubyte *compressed_data = (GLubyte*)malloc((video_buffer_size)*sizeof(GLubyte));

	int i = 0;
	int r;
	while (!glfwWindowShouldClose(g_win)) {
		glfwPollEvents();
		g_retro.retro_run();
		glClear(GL_COLOR_BUFFER_BIT);
		video_render();

		if (i%6 == 0) {
			glReadPixels(0, 0, nwidth, nheight, GL_BGR, GL_UNSIGNED_BYTE, data);

			r = compress2(compressed_data, &compressed_size, data, video_buffer_size, 1);
			if (r == Z_BUF_ERROR) cout << "Buffer not big enough\n";

			if (auto dc = weak_dc.lock()) dc->send((byte*)compressed_data, compressed_size);
			i=0;
			compressed_size = video_buffer_size;
		}
		i++;
		glfwSwapBuffers(g_win);
	}

	free(data);
	free(compressed_data);
	core_unload();
	audio_deinit();
	video_deinit();
	xdo_free(xwin);

	glfwTerminate();

	cout << "Cleaning up..." << endl;

	clients.clear();
	delete params;
	return 0;
}

// Create and setup a PeerConnection
shared_ptr<Client> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string from_id,
												string to_id, string localId) {
	auto pc = make_shared<PeerConnection>(config);
	shared_ptr<Client> client(new Client(pc));

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

	// We are the offerer, so create a data channel to initiate the process
	const string label = "gameChannel";
	cout << "Creating DataChannel with label \"" << label << "\"" << endl;
	auto dc = pc->createDataChannel(label);
	client->dataChannel = dc;

	clients.emplace(to_id, client);
	return client;
};
