import express from 'express';
import {WebSocket, WebSocketServer} from 'ws';
import http from 'http';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

import {toggleLight, getLightStatus} from './apiFunctions.mjs'

const clients = [];
var micro_conn = null;

wss.on('connection', (ws) => {
  console.log('Client connected');
  
  ws.on('message', (message) => {
    message = message.toString();
    console.log(`Received: ${message}`); 
      
    switch(message) {
      case "webpage":
        clients.push(ws);
        console.log(`New webpage connection (total: ${clients.length})`);
        break;

      case "micro":
        micro_conn = ws;
        console.log('Micro connected')
        break;

      case "hello": 
        ws.send(`Hi There`);
        break;

      case "toggle light":
        toggleLight((err, ledStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${ledStatus}`);
            ws.send(`ledStatus: ${ledStatus}`);
          }
        }); 
        break;

      case "get light status":
        getLightStatus((err, ledStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`${ledStatus}`);
            ws.send(ledStatus);
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

