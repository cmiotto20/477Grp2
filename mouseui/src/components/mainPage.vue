<template>
  <h1>MOUSE Controls Main Page</h1>
  <button @click="turnOnLight()">Toggle light</button>
</template>

<script>
export default {
  name: 'mainPage',
  props: {
    msg: String
  },
  methods: {
    turnOnLight() {
<<<<<<< HEAD
      const url = 'http://174.129.215.96:8080/toggleLight'; 
=======
      const url = 'http://52.23.199.138:8080/toggleLight'; 
>>>>>>> 30645964d247636041cdd28efe9567316fbc376a

      fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json', 
        },
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
    }
  }
}
</script>

