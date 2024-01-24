<template>
  <h1>MOUSE Controls Main Page</h1>
  <button @click="toggleLight()">Toggle light</button>
  <button @click="moveAction('U')">Up</button>
  <button @click="moveAction('R')">Right</button>
  <button @click="moveAction('L')">Left</button>
  <button @click="moveAction('D')">Down</button>
  <button @click="sendMessage()">Send Message</button>
</template>

<script>
export default {
  name: 'mainPage',
  data() {
    return {
      messages: [],
      socket: null,
    };
  },
  props: {
    msg: String
  },
  methods: {
    moveAction(dir) {
      this.socket.send("[mv " + dir + "]")
    },
    sendMessage() {
      this.socket.send("[hello]");
    },
    toggleLight() {
      this.socket.send("[toggle light]");
    },
    getDataStream(event){
      const regexData = /:\s*([\s\S]*)$/; //matches data stream into array 
      const result = event.data.match(regexData);
      if(!result || result.length <= 1){
        console.log('invalid data parse');
      } else {
        return result[1].trim().split(/\s+/); //separates elements by whitespace
      }
    },
  },
  mounted() {
    this.socket = new WebSocket('ws://174.129.215.96:3000');
    //this.socket = new WebSocket('ws://localhost:3000');
    //wait for socket connection to be established
    this.socket.onopen = () => {
      this.socket.send("[webpage]");
      console.log("Connection msg sent");
    };
    this.socket.onmessage = (event) => {
      this.messages.push(event.data);
      console.log(event.data);
      
      //parse data received in message
      const regexCmd = /\[([^\]]+)\]/; //matches items between square brackets
      const result = event.data.match(regexCmd);
      console.log(`result: ${result}`);
      //check if regex match
      if(!result || result.length <= 1){
        console.log("Error: could not parse [command] format");
      }
      const command = result[1];
      console.log(`command: ${command}`);

      switch(command) {
        case "detected": {
          const time = this.getDataStream(event);
          alert(`Motion Detected! [${time}]`);
          console.log(`Motion Detected! [${time}]`); 
          break;
        }
        
        case "ledStatus": {
          const status = this.getDataStream(event);
          console.log(`LED Status: ${status}`);
          break;
        }
        
        case "toggleLight": {
          const status = this.getDataStream(event);
          console.log(`LED Status: ${status}`);
          break;
        }

        case "hello": {
          const hellomsg = this.getDataStream(event);
          console.log(`hello msg: ${hellomsg}`); 
          break;
        } 

        default:
          console.log('Error: invalid command read');
      }
    };
    
  }
}
</script>

