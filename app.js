import express from 'express';
import {WebSocket, WebSocketServer} from 'ws';
import http from 'http';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

import {toggleRow, getRowStatus, setRow, increaseRow} from './apiFunctions.mjs'

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
      // console.log("Error: could not parse [command] format");
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
        const currentTime = new Date();
        setRow(1, currentTime, (err, microStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${microStatus}`);
            ws.send(`[micro]: ${microStatus}`);
          }
        }); 
        break;
      
      case "micro_conn": {
        getRowStatus(1, (err, microStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            const currentTime = new Date();
            const microTime = new Date(microStatus);
            const timeDifference = Math.abs(currentTime - microTime);
            console.log(`Got last micro time of ${microTime}`)
            console.log(`Time difference: ${timeDifference}`);
            if (timeDifference > 10000) {
              console.log("The time difference is more than 10 seconds.");
              console.log(`Result: ${0}`);
              ws.send(`[micro_conn]: ${0}`);
            } else {
              console.log("The time difference is within 10 seconds.");
              console.log(`Result: ${1}`);
              ws.send(`[micro_conn]: ${1}`);
            }
          }
        }); 
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

      case "gL": // Get Light Status
        getRowStatus(0, (err, ledStatus) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`${ledStatus}`);
            ws.send(`[ledStatus]: ${ledStatus}`);
          }
        }); 
        break;

      case "gD": // Get Motor Direction
        getRowStatus(2, (err, motorDirection) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`${motorDirection}`);
            ws.send(`${motorDirection}`);
          }
        }); 
        break;

      case "mv R":
        console.log("Received move R command");
        increaseRow(2, 'R', (err, motorDirectionSpeed) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${motorDirectionSpeed}`);
            ws.send(`[motor]: ${motorDirectionSpeed}`);
          }
        }); 
        break;
      
      case "mv L":
        console.log("Received move L command");
        increaseRow(2, 'L', (err, motorDirectionSpeed) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${motorDirectionSpeed}`);
            ws.send(`[motor]: ${motorDirectionSpeed}`);
          }
        }); 
        break;

      case "mv U":
        console.log("Received move U command");
        increaseRow(2, 'U', (err, motorDirectionSpeed) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${motorDirectionSpeed}`);
            ws.send(`[motor]: ${motorDirectionSpeed}`);
          }
        }); 
        break;

      case "mv D":
        console.log("Received move D command");
        increaseRow(2, 'D', (err, motorDirectionSpeed) => {
          if (err) {
            console.error(`Error: ${err}`);
          } else {
            console.log(`Result: ${motorDirectionSpeed}`);
            ws.send(`[motor]: ${motorDirectionSpeed}`);
          }
        }); 
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

