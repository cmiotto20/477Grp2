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
      this.socket.send("mv " + dir)
    },
    sendMessage() {
      this.socket.send("hello");
    },
    toggleLight() {
      this.socket.send("toggle light");
    }
  },
  mounted() {
    //this.socket = new WebSocket('ws://174.129.215.96:3000');
    this.socket = new WebSocket('ws://localhost:3000');
    //wait for socket connection to be established
    this.socket.onopen = () => {
      this.socket.send("webpage");
      console.log("Connection msg sent");
    };
    this.socket.onmessage = (event) => {
      this.messages.push(event.data);
      console.log(event.data)
    };
  }
}
</script>

