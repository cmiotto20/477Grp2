<template>
  <div class="container">
    <h1>MOUSE Controls Main Page</h1>
    <button @click="toggleLight()">Toggle light</button>
    <button @click="sendMessage()">Send Message</button>

    <!-- Microcontroller connection button -->
    <button class="connected-button" :class="{ 'connected': micro_conn, 'not-connected': !micro_conn }"> Microcontroller Connection</button>

    <!-- Joypad layout for arrow buttons -->
    <div class="joypad-container">
      <div class="arrow-button-row">
        <div class="arrow-button-container">
          <button @click="moveAction('U')" class="arrow-button arrow-up"></button>
        </div>
      </div>

      <div class="arrow-button-row">
        <div class="arrow-button-container">
          <button @click="moveAction('L')" class="arrow-button arrow-left"></button>
        </div>

        <div class="arrow-button-container">
          <button @click="moveAction('R')" class="arrow-button arrow-right"></button>
        </div>
      </div>

      <div class="arrow-button-row">
        <div class="arrow-button-container">
          <button @click="moveAction('D')" class="arrow-button arrow-down"></button>
        </div>
      </div>
    </div>

  </div>
</template>

<script>
export default {
  name: 'mainPage',
  data() {
    return {
      messages: [],
      socket: null,
      micro_conn: false,
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
      setTimeout(() => {
        this.socket.send("[micro_conn]");
        console.log("Connection msg sent");
      }, 500);
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

        case "micro_conn": {
          this.micro_conn = parseInt(this.getDataStream(event)) == 1 ? true : false; 
          console.log(`micro_conn received: ${this.micro_conn}`);
          break;
        }

        default:
          console.log('Error: invalid command read');
      }
    };
    
  }
}
</script>

<style>
  /* Import the styles.css file */
  @import './style.css'; 
</style>
