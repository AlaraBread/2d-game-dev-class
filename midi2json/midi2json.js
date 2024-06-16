#!/usr/bin/env node

var fs = require('fs');
var parseMidi = require('midi-file').parseMidi;
var writeMidi = require('midi-file').writeMidi;

if(process.argv.length <= 1+3) {
	console.log("provide a filename, song filename, bpm, title, and jump velocity on the commandline.");
	process.exit();
}

let input = fs.readFileSync(process.argv[2]);
let parsed = parseMidi(input);

let ticksPerBeat = parsed.header.ticksPerBeat;

let beats = [];
let secondary_beats = [];
let enemies = [];
let xPos = 300;
let endBeat = 99999999999999999.0;

for(let track of parsed.tracks) {
	let time = 0;
	for(let e of track) {
		time += e.deltaTime;
		if(e.type === "noteOn") {
			if(e.noteNumber === 60) {
				beats.push(time/ticksPerBeat);
			} else if(e.noteNumber === 61) {
				secondary_beats.push(time/ticksPerBeat);
			} else if (e.noteNumber === 59){
				endBeat = time/ticksPerBeat;
			} else {
				enemies.push({
					"time": time/ticksPerBeat,
					"type": Math.floor(e.noteNumber/2.0)-Math.floor(62.0/2.0),
					"position_x": xPos,
					"position_y": 100.0,
					"velocity_x": -400.0*Math.sign(xPos-600),
					"velocity_y": 400.0,
				});
				xPos = xPos === 300 ? 900 : 300;
			}
		}
	}
}

level = {
	"title": process.argv[5],
	"bpm": parseFloat(process.argv[4]),
	"end": endBeat,
	"song": process.argv[3],
	"jump_velocity": parseFloat(process.argv[6]),
	"beats": beats,
	"secondary_beats": secondary_beats,
	"enemy_spawns": enemies,
};

fs.writeFileSync("level.json", JSON.stringify(level));
