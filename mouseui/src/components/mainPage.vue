<template>
  <h1>MOUSE Controls Main Page</h1>
  <button @click="turnOnLight()">Toggle light</button>
</template>

<script>
export default {
  name: 'HelloWorld',
  props: {
    msg: String
  },
  methods: {
    turnOnLight() {
      const url = 'http://54.226.108.25:8080/toggleLight'; 

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

