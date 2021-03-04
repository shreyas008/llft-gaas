from flask import Flask, jsonify, request, json
from flask import g
import sqlite3

app = Flask(__name__)
DATABASE = 'GAME_USERS.db'

def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(DATABASE)
    return db

@app.route("/login",methods=['POST'])
def login():
    db = get_db()
    cur = db.cursor()
    email = request.form.get('email')
    password = request.form.get('password')
    #try:
    #        check = cur.execute("INSERT into Users (email,pass) values (?,?)",(email,password))
    #except:
    #        return jsonify({'status': 'User already exists'})
    #db.commit()
    db.close()
    return jsonify({'status': 'success'})

if __name__ == "__main__":
    con = sqlite3.connect(DATABASE)  
    con.execute("create table if not exists Users (email TEXT UNIQUE PRIMARY KEY, pass TEXT NOT NULL)")
    con.close()

    app.run(host='0.0.0.0',debug=True, ssl_context='adhoc')
