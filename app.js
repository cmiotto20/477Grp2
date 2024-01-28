import express from 'express';
import {WebSocket, WebSocketServer} from 'ws';
import http from 'http';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

import {toggleRow, getLightStatus} from './apiFunctions.mjs'

//declaring global variables to track socket clients
const clients = [];
var micro_conn = null;

function broadcastMsg(msg){
  for (const client of clients){
    client.send(msg);
  }
}

function processUltrasonic(datastream){
  //TODO: Process ultrasonic data stream and report movement detection
  // if alert, call movementAlert()
}

function movementAlert(){
  const currentDate = new Date();
  const formatTime = currentDate.toLocaleString();
  //TODO: add motion detection data to db file
  console.log(`Detected movement at ${formatTime}`);
  const msg = `[detected]: ${formatTime}`;
  broadcastMsg(msg);
}

wss.on('connection', (ws) => {
  console.log('Client connected');
  
  ws.on('message', (message) => {
    message = message.toString();
    const regex = /\[([^\]]+)\]/; //matches items between square brackets
    const result = message.match(regex);
    //check if regex match
    let command;
    if(!result || result.length <= 1){
      console.log("Error: could not parse [command] format");
      command = message;
    } else {
      command = result[1];
    }

    console.log(`Received: ${message}`); 
    console.log(`Command: ${command}`);
      
    switch(command) {
      case "webpage":
        clients.push(ws);
        console.log(`New webpage connection (total: ${clients.length})`);
        break;

      case "m":
        micro_conn = 1;
        console.log('Micro connected')
        ws.send(`[micro]: 1`);
        break;
      
      case "micro_conn": {
        const conn_status = micro_conn;
        ws.send(`[micro_conn]: ${conn_status}`);
        break;
      }

      case "hello": 
        //ws.send(`[hello]: Hi There`);
        movementAlert();
        break;

      case "toggle light":
        toggleRow(0, (err, ledStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${ledStatus}`);
            ws.send(`[toggleLight]: ${ledStatus}`);
          }
        }); 
        break;

      case "gL":
        getLightStatus((err, ledStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`${ledStatus}`);
            ws.send(`[ledStatus]: ${ledStatus}`);
          }
        }); 
        break;

      case "mv R":
        console.log("Received move R command");
        if(micro_conn !== null){
          console.log("Sending move R to micro");
          //can send data to microcontroller connection here 
          //i.e. micro_conn.send('R');
          return;
        }
        console.log("No micro connection");
        break;
      
      case "mv L":
        console.log("Received move L command");
        if(micro_conn !== null){
          console.log("Sending move L to micro");
          //can send data to microcontroller connection here 
          //i.e. micro_conn.send('L');
          return;
        }
        console.log("No micro connection");
        break;

      case "mv U":
        console.log("Received move U command");
        if(micro_conn !== null){
          console.log("Sending move U to micro");
          //can send data to microcontroller connection here 
          //i.e. micro_conn.send('U');
          return;
        }
        console.log("No micro connection");
        break;

      case "mv D":
        console.log("Received move D command");
        if(micro_conn !== null){
          console.log("Sending move D to micro");
          //can send data to microcontroller connection here 
          //i.e. micro_conn.send('D');
          return;
        }
        console.log("No micro connection");
        break;

      default: 
        console.log('Error: invalid socket read');
    }
  });

  ws.on('close', () => {
      //check if removing webpage client
      const client_idx = clients.indexOf(ws);
      if(client_idx !== -1){
        clients.splice(client_idx, 1);
        console.log(`Webpage client disconnected (total: ${clients.length})`);
        return;
      }
      //otherwise check if removing micro client
      if(micro_conn == ws){
        console.log('Microcontroller client disconnected');
        micro_conn = null;
        broadcastMsg('[micro]: 0');
        return;
      }
      console.log('Error: closing unidentified connection');
  });
});


app.get('/move', (req, res) => { 
    console.log("req: " + req)
    console.log('sending movement: Up'); 
    
    res.status(200).send(`U`);
  });

server.listen(3000, () => {
    console.log('Server listening on port 3000');
});

