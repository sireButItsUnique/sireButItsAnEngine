const https = require("https");
const axios = require("axios");
const { spawn } = require("child_process");
require("dotenv").config();

async function listenGame(gameId, color) {
	// init engine
	let engine = spawn("../engine.exe", {
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
			if (terms[0] != "enginemove" && terms[0] != "engineeval") {
				console.log(data);
			}

			// engine wants to play a move
			if (terms[0] == "enginemove") {
				console.log(
					`engine plays /api/board/game/${gameId}/move/${terms[1]}`
				);
				axios
					.post(
						`https://lichess.org/api/bot/game/${gameId}/chat/`,
						{
							room: "player",
							text: `${terms[2]}`,
						},
						{
							headers: {
								Authorization: `Bearer ${process.env.API_TOKEN}`,
							},
						}
					)
					.catch((err) => {
						console.error(err.response.data);
					});
				axios
					.post(
						`https://lichess.org/api/bot/game/${gameId}/chat/`,
						{
							room: "spectator",
							text: `${terms[2]}`,
						},
						{
							headers: {
								Authorization: `Bearer ${process.env.API_TOKEN}`,
							},
						}
					)
					.catch((err) => {
						console.error(err.response.data);
					});
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
			} else if (terms[0] == "engineeval") {
				console.log("eval: " + terms[1]);
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
				console.log("ORG:");
				console.log(json);
				let entries = json.split("\n");

				for (let i = 0; i < entries.length; i++) {
					if (!entries[i].trim()) {
						continue;
					}
					console.log("ENTRIES:");
					console.log(entries[i]);
					json = JSON.parse(entries[i]);
					let state = false;

					// chat reply
					if (
						json.type == "chatLine" &&
						json.username != "sireButItsAnEngine"
					) {
						axios
							.post(
								`https://lichess.org/api/bot/game/${gameId}/chat/`,
								{
									room: "player",
									text: "fuck off im tryna think",
								},
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

					// doing game state only
					else if (json.type == "gameFull") {
						state = json.state;
					} else if (json.type == "gameState") {
						state = json;
					}

					if (state) {
						// game end
						if (state.status != "started") {
							engine.stdin.end("quit ");
							return;
						}

						// new move
						let moves = state.moves.split(" ");
						if (state.moves != "") {
							engine.stdin.write(
								`lichessmove ${moves[moves.length - 1]} print `
							);
						}

						// determine if its the engines turn to play
						if (
							color == "black" &&
							moves.length % 2 == 1 &&
							moves[0] != ""
						) {
							engine.stdin.write("lichesseval ");
						} else if (
							color == "white" &&
							(moves.length % 2 == 0 || moves[0] == "")
						) {
							engine.stdin.write("lichesseval ");
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

console.log(process.env.API_TOKEN)
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
