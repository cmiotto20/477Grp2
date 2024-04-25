<template>
  <div class="container">
    <div id="controlBoxTitle">mouse Controls Main Page</div>
    <div id="containerBody">
      <div id="leftHalf">
        <div class="outerBtnGroup">
          <div class="innerBtnGroup">
            <button @click="recordInputs()" class="btnControls" :class="{'live':recording,}">Start Record</button>
            <button @click= "playbackInputs()" class="btnControls" :class="{'live':playback,}">Start Playback</button>
          </div>
          <div class ="innerBtnGroup">
            <button @click="stopRecordInputs()" class="btnControls">Stop Record</button>
            <button @click="stopPlayback()" class="btnControls">Stop Playback</button>
          </div>
          <div class="innerBtnGroup">
            <button @click="toggleLight()" class="btnControls">Toggle light</button>
            <button @click="sendMessage()" class="btnControls">Send Message</button>
          </div>

          <!-- Microcontroller connection button -->
          <button class="connected-button" :class="{ 'connected': micro_conn, 'not-connected': !micro_conn }"> Microcontroller Connection</button>
          <button id="state_container" :class="{ 'scanning': micro_state, 'manual': !micro_state }"> {{ state_message }} </button>
        </div>

        <!-- Joypad layout for arrow buttons -->
        <div class="joypad-container">
          <div class="arrow-button-row">
            <div class="arrow-button-container">
              <button @click="moveAction('U')" class="arrow-button arrow-up">▲</button>
            </div>
          </div>

          <div class="arrow-button-row">
            <div class="arrow-button-container">
              <button @click="moveAction('L')" class="arrow-button arrow-left">▲</button>
            </div>

            <div class="arrow-button-container">
              <button @click="moveAction('R')" class="arrow-button arrow-right">▲</button>
            </div>
          </div>

          <div class="arrow-button-row">
            <div class="arrow-button-container">
              <button @click="moveAction('D')" class="arrow-button arrow-down">▲</button>
            </div>
          </div>

          <button @click="moveAction('S')" id="stopBtn">STOP</button>

        </div>
      </div>
      <div id="rightHalf">
        <div id="messageBox">
          <div class="message" v-for="message in messages_for_message_box" :key="message">
            {{ message }}
          </div>
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
      messages_for_message_box: [],
      recording: false,
      playback: false,
      micro_state: 0,
      state_message: "Manual Movement"
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
    recordInputs() {
      console.log("hit record");
      this.socket.send("[record]");
    },
    playbackInputs(){
      console.log("hit playback");
      this.socket.send("[playback]");
    },
    stopRecordInputs(){
      console.log("hit stop record");
      this.socket.send("[done rec]");
    },
    stopPlayback(){
      console.log("stop playback");
      this.socket.send("[stp play]");
    }
  },
  mounted() {
    this.socket = new WebSocket('ws://174.129.215.96:3000');
    //this.socket = new WebSocket('ws://localhost:3000');

    //wait for socket connection to be established
    this.socket.onopen = () => {
      this.socket.send("[webpage]");
      console.log("Connection msg sent");

      this.socket.send("[micro_conn]");
      console.log("Checking for micro connection msg sent");

      this.socket.send("[checkMovementDetection]");
      console.log("Checking for movement detection");

      this.socket.send("[state]");
      console.log("Checking micro state");
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
          var time_msg = ""
          for (const timecomp of time){
            if(timecomp.trim() === "") continue;
            time_msg += timecomp.trim() + " "
          }
          this.messages_for_message_box.push(`Motion Detected! ${time_msg}`);
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

        case "motor": {
          const motormsg = this.getDataStream(event);
          console.log(`motor msg: ${motormsg}`); 
          break;
        } 

        case "micro_conn": {
          console.log(`micro_conn received: ${this.micro_conn}`);
          break;
        }

        case "state": {
          this.micro_state = parseInt(this.getDataStream(event)[0]); 
          console.log(`micro_state received: ${this.micro_state}`);
          switch(this.micro_state) {
            case 0:
              this.state_message = "Manual Movement";
              break;
            case 1:
              this.state_message = "Scanning";
              break;
            case 2:
              this.state_message = "Playback";
              break;
          }

          break;
        }

        case "movementDetection": {
          let movement = (event.data).substring(21);
          console.log(`movement: ${movement}`)
          movement = `[${movement}]`
          let processedString = movement.replace(/,/g, '","');
          processedString = processedString.replace(/\[/g, '["');
          processedString = processedString.replace(/\]/g, '"]');
          let valArr = JSON.parse(processedString); 
          console.log(`movement detection received: ${valArr}`);
          valArr.forEach((time) => {
            if(time != "") {
              this.messages_for_message_box.push(`Motion Detected at ${time}!`);
            } 
          });
          break;
        }

        case "p/r err": {
          alert("Cannot playback and record simultaneously! Press stop on running process");
          console.log("playback/recording error");
          break;
        }

        case "recording status": {
          console.log("received recording status");
          this.recording = parseInt(this.getDataStream(event)) == 1 ? true : false;
          break;
        }

        case "playback status": {
          console.log("received playback status");
          this.playback = parseInt(this.getDataStream(event)) == 1 ? true : false;
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
