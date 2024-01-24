(function(){"use strict";var t={5683:function(t,o,n){var e=n(9242),r=n(3396);function c(t,o,n,e,c,s){const i=(0,r.up)("mainPage");return(0,r.wg)(),(0,r.j4)(i)}const s=(0,r._)("h1",null,"MOUSE Controls Main Page",-1),i={key:0},a={key:1};function l(t,o,n,e,c,l){return(0,r.wg)(),(0,r.iD)(r.HY,null,[s,(0,r._)("button",{onClick:o[0]||(o[0]=t=>l.toggleLight())},"Toggle light"),(0,r._)("button",{onClick:o[1]||(o[1]=t=>l.moveAction("U"))},"Up"),(0,r._)("button",{onClick:o[2]||(o[2]=t=>l.moveAction("R"))},"Right"),(0,r._)("button",{onClick:o[3]||(o[3]=t=>l.moveAction("L"))},"Left"),(0,r._)("button",{onClick:o[4]||(o[4]=t=>l.moveAction("D"))},"Down"),(0,r._)("button",{onClick:o[5]||(o[5]=t=>l.sendMessage())},"Send Message"),this.micro_conn?((0,r.wg)(),(0,r.iD)("h2",i,"Microcontroller Connected")):((0,r.wg)(),(0,r.iD)("h2",a," No Microcontroller Connection "))],64)}n(560);var u={name:"mainPage",data(){return{messages:[],socket:null,micro_conn:!1}},props:{msg:String},methods:{moveAction(t){this.socket.send("[mv "+t+"]")},sendMessage(){this.socket.send("[hello]")},toggleLight(){this.socket.send("[toggle light]")},getDataStream(t){const o=/:\s*([\s\S]*)$/,n=t.data.match(o);if(n&&!(n.length<=1))return n[1].trim().split(/\s+/);console.log("invalid data parse")}},mounted(){this.socket=new WebSocket("ws://174.129.215.96:3000"),this.socket.onopen=()=>{this.socket.send("[webpage]"),this.socket.send("[micro_conn]"),console.log("Connection msg sent")},this.socket.onmessage=t=>{this.messages.push(t.data),console.log(t.data);const o=/\[([^\]]+)\]/,n=t.data.match(o);console.log(`result: ${n}`),(!n||n.length<=1)&&console.log("Error: could not parse [command] format");const e=n[1];switch(console.log(`command: ${e}`),e){case"detected":{const o=this.getDataStream(t);alert(`Motion Detected! [${o}]`),console.log(`Motion Detected! [${o}]`);break}case"ledStatus":{const o=this.getDataStream(t);console.log(`LED Status: ${o}`);break}case"toggleLight":{const o=this.getDataStream(t);console.log(`LED Status: ${o}`);break}case"hello":{const o=this.getDataStream(t);console.log(`hello msg: ${o}`);break}case"micro_conn":this.micro_conn=1==parseInt(this.getDataStream(t)),console.log(`micro_conn received: ${this.micro_conn}`);break;default:console.log("Error: invalid command read")}}}},g=n(89);const h=(0,g.Z)(u,[["render",l]]);var m=h,f={name:"App",components:{mainPage:m}};const d=(0,g.Z)(f,[["render",c]]);var p=d;(0,e.ri)(p).mount("#app")}},o={};function n(e){var r=o[e];if(void 0!==r)return r.exports;var c=o[e]={exports:{}};return t[e].call(c.exports,c,c.exports,n),c.exports}n.m=t,function(){var t=[];n.O=function(o,e,r,c){if(!e){var s=1/0;for(u=0;u<t.length;u++){e=t[u][0],r=t[u][1],c=t[u][2];for(var i=!0,a=0;a<e.length;a++)(!1&c||s>=c)&&Object.keys(n.O).every((function(t){return n.O[t](e[a])}))?e.splice(a--,1):(i=!1,c<s&&(s=c));if(i){t.splice(u--,1);var l=r();void 0!==l&&(o=l)}}return o}c=c||0;for(var u=t.length;u>0&&t[u-1][2]>c;u--)t[u]=t[u-1];t[u]=[e,r,c]}}(),function(){n.n=function(t){var o=t&&t.__esModule?function(){return t["default"]}:function(){return t};return n.d(o,{a:o}),o}}(),function(){n.d=function(t,o){for(var e in o)n.o(o,e)&&!n.o(t,e)&&Object.defineProperty(t,e,{enumerable:!0,get:o[e]})}}(),function(){n.g=function(){if("object"===typeof globalThis)return globalThis;try{return this||new Function("return this")()}catch(t){if("object"===typeof window)return window}}()}(),function(){n.o=function(t,o){return Object.prototype.hasOwnProperty.call(t,o)}}(),function(){var t={143:0};n.O.j=function(o){return 0===t[o]};var o=function(o,e){var r,c,s=e[0],i=e[1],a=e[2],l=0;if(s.some((function(o){return 0!==t[o]}))){for(r in i)n.o(i,r)&&(n.m[r]=i[r]);if(a)var u=a(n)}for(o&&o(e);l<s.length;l++)c=s[l],n.o(t,c)&&t[c]&&t[c][0](),t[c]=0;return n.O(u)},e=self["webpackChunkmouseui"]=self["webpackChunkmouseui"]||[];e.forEach(o.bind(null,0)),e.push=o.bind(null,e.push.bind(e))}();var e=n.O(void 0,[998],(function(){return n(5683)}));e=n.O(e)})();
//# sourceMappingURL=app.216bb893.js.map