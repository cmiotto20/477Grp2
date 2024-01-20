<template>
  <h1>MOUSE Controls Main Page</h1>
  <button @click="turnOnLight()">Toggle light</button>
  <button @click="moveUp()">Up</button>
  <button @click="moveRight()">Right</button>
  <button @click="moveLeft()">Left</button>
  <button @click="moveDown()">Down</button>
  <button @click="sendMessage()">Send Message</button>
</template>

<script>
//const home = 'http://174.129.215.96:8080/'
const home = 'http://localhost:8080/';
export default {
  name: 'mainPage',
  data() {
    return {
      websocketStatus: 'Disconnected',
    };
  },
  props: {
    msg: String
  },
  methods: {
    turnOnLight() {
      const url = 'http://174.129.215.96:8080/toggleLight'; 

      fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json', 
        },
      })
        .then(data => {
          console.log('Response:', data);
        })
        .catch(error => {
          console.error('Error:', error);
        });
    },
    moveUp() {
      const url = home + 'moveUp'
      fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json', 
        },
        body: JSON.stringify({dir : "U"}),
      })
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.text();
        })
        .then(data => {
          console.log('Response:', data);
        })
        .catch(error => {
          console.error('Error:', error);
        });
    }, 
    moveLeft() {
      const url = home + 'moveLeft'
      fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json', 
        },
        body: JSON.stringify({dir : "L"}),
      })
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.text();
        })
        .then(data => {
          console.log('Response:', data);
        })
        .catch(error => {
          console.error('Error:', error);
        });
    },
    moveDown() {
      const url = home + 'D'
      fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json', 
        },
        body: JSON.stringify({dir : "D"}),
      })
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.text();
        })
        .then(data => {
          console.log('Response:', data);
        })
        .catch(error => {
          console.error('Error:', error);
        });
    },
    moveRight() {
      const url = home + 'R'
      fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json', 
        },
        body: JSON.stringify({dir : "R"}),
      })
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.text();
        })
        .then(data => {
          console.log('Response:', data);
        })
        .catch(error => {
          console.error('Error:', error);
        });
    },
    sendMessage() {
      this.$socket.send('Hello, server!');
    }
  },
  mounted() {
    this.$options.sockets.onopen = () => {
      this.websocketStatus = 'Connected';
      console.log('WebSocket connected');
    };

    this.$options.sockets.onclose = () => {
      this.websocketStatus = 'Disconnected';
      console.log('WebSocket disconnected');
    };

    this.$options.sockets.onmessage = (message) => {
      console.log('Message from server:', message);
      // Handle the received data as needed
    };
  }
}
</script>

