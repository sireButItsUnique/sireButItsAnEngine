const https = require("https");
const axios = require("axios");
const { spawn } = require("child_process");
require("dotenv").config();

async function listenGame(gameId, color) {
	// init engine
	let engine = spawn("../build/Engine.exe", {
		stdio: ["pipe", "pipe", "pipe"],
	});

	engine.on("exit", (code, signal) => {
		console.log(`${gameId} exited with code ${code}`);
	});

	// engine move output
	engine.stdout.on("data", (data) => {
		data = data.toString("utf-8");
		if (data.trim()) {
			let terms = data.split(" ");
			console.log(data);

			// engine wants to play a move
			if (terms[0] == "bestmove") {
				axios
					.post(
						`https://lichess.org/api/bot/game/${gameId}/move/${terms[1]}`,
						{},
						{
							headers: {
								Authorization: `Bearer ${process.env.API_TOKEN}`,
							},
						}
					)
					.catch((err) => {
						console.error(err.response.data);
					});
			}
		}
	});

	// game data stream
	let req = https.request(
		{
			hostname: "lichess.org",
			path: "/api/bot/game/stream/" + gameId,
			method: "GET",
			headers: {
				Authorization: `Bearer ${process.env.API_TOKEN}`,
			},
		},
		(res) => {
			res.on("data", (data) => {
				let json = data.toString("utf-8");
				if (!json.trim()) {
					return;
				}
				let entries = json.split("\n");

				for (let i = 0; i < entries.length; i++) {
					if (!entries[i].trim()) continue;
                    json = JSON.parse(entries[i]);

					// doing game state only
                    let state = false;
					if (json.type == "gameFull") {
						state = json.state;
					} else if (json.type == "gameState") {
						state = json;
					}
                    console.log(state);
                    if (!state) continue;

                    // game end
                    if (state.status != "started") {
                        engine.stdin.end("quit\n");
                        return;
                    }

					if (state.status == "started") {
						
						// new move
						let moves = state.moves

						// determine if its the engines turn to play
                        let colorToPlay = moves.split(" ").length % 2 == 0 ? "white" : "black";
                        if (colorToPlay == color) {
                            engine.stdin.write(`position startpos moves ${moves}\n`);
                            engine.stdin.write(`go wtime ${state.wtime} btime ${state.btime} winc ${state.winc} binc ${state.binc}\n`);
                        }
					}
				}
			});
		}
	);

	req.end();
}

async function listenEvents() {
	let req = https.request(
		{
			hostname: "lichess.org",
			path: "/api/stream/event",
			method: "GET",
			headers: {
				Authorization: `Bearer ${process.env.API_TOKEN}`,
			},
		},
		(res) => {
			res.on("data", (data) => {
				let json = data.toString("utf-8");
				if (json.trim()) {
					json = JSON.parse(json);
					console.log(json.type);

					// challenge issued
					if (json.type == "challenge") {
                        console.log("Challenge accepted: " + json.challenge.id);
						axios.post(
							`https://lichess.org/api/challenge/${json.challenge.id}/accept`,
							{},
							{
								headers: {
									Authorization: `Bearer ${process.env.API_TOKEN}`,
								},
							}
						);
					}

					// accept challenge
					if (json.type == "gameStart") {
						listenGame(json.game.gameId, json.game.color);
					}
				}
			});
		}
	);

	req.end();
}

listenEvents();

// weird rules to use for engine:
// spawn engine ONCE
// do NOT call writeable.end() until game ends
// let engine = spawn("../engine.exe");

// engine.on("exit", (code, signal) => {
// 	console.log(`${gameId} exited with code ${code}`);
// });

// engine.stdout.on("data", (data) => {
// 	data = data.toString("utf-8");
// 	if (data.trim()) {
// 		console.log("engine says " + data);
// 	}
// });

// engine.stdin.write("lichessmove e2e4 print");
// engine.stdin.write(" ");

// engine.stdin.write("lichessmove g8h6 print lichesseval");
// engine.stdin.write(" ");