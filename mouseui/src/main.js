import { createApp } from 'vue'
import App from './App.vue'
import VueNativeSock from 'vue-native-websocket';


createApp(App).mount('#app')

const websocketOptions = { format: 'json' };
createApp(App).use(VueNativeSock, 'ws://174.129.215.96:8080', websocketOptions);
createApp(App).mount('#app');