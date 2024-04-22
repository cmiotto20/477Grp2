(function(){"use strict";var t={3591:function(t,o,e){var n=e(9242),s=e(3396);function a(t,o,e,n,a,r){const c=(0,s.up)("mainPage");return(0,s.wg)(),(0,s.j4)(c)}var r=e(7139);const c={class:"container"},i=(0,s._)("div",{id:"controlBoxTitle"},"mouse Controls Main Page",-1),l={id:"containerBody"},u={id:"leftHalf"},g={class:"outerBtnGroup"},d={class:"innerBtnGroup"},m={class:"innerBtnGroup"},b={class:"innerBtnGroup"},h={class:"joypad-container"},p={class:"arrow-button-row"},k={class:"arrow-button-container"},_={class:"arrow-button-row"},v={class:"arrow-button-container"},f={class:"arrow-button-container"},w={class:"arrow-button-row"},y={class:"arrow-button-container"},C={id:"rightHalf"},S={id:"messageBox"};function D(t,o,e,n,a,D){return(0,s.wg)(),(0,s.iD)("div",c,[i,(0,s._)("div",l,[(0,s._)("div",u,[(0,s._)("div",g,[(0,s._)("div",d,[(0,s._)("button",{onClick:o[0]||(o[0]=t=>D.recordInputs()),class:(0,r.C_)(["btnControls",{live:a.recording}])},"Start Record",2),(0,s._)("button",{onClick:o[1]||(o[1]=t=>D.startPlayback()),class:(0,r.C_)(["btnControls",{live:a.playback}])},"Start Playback",2)]),(0,s._)("div",m,[(0,s._)("button",{onClick:o[2]||(o[2]=t=>D.stopRecordInputs()),class:"btnControls"},"Stop Record"),(0,s._)("button",{onClick:o[3]||(o[3]=t=>D.stopPlayback()),class:"btnControls"},"Stop Playback")]),(0,s._)("div",b,[(0,s._)("button",{onClick:o[4]||(o[4]=t=>D.toggleLight()),class:"btnControls"},"Toggle Light"),(0,s._)("button",{onClick:o[5]||(o[5]=t=>D.initiateScan()),class:"btnControls"},"Toggle Scan")]),(0,s._)("button",{class:(0,r.C_)(["connected-button",{connected:a.micro_conn,"not-connected":!a.micro_conn}])}," Microcontroller Connection",2),(0,s._)("button",{id:"state_container",class:(0,r.C_)({scanning:a.micro_state,manual:!a.micro_state})},(0,r.zw)(a.state_message),3)]),(0,s._)("div",h,[(0,s._)("div",p,[(0,s._)("div",k,[(0,s._)("button",{onClick:o[6]||(o[6]=t=>D.moveAction("U")),class:"arrow-button arrow-up"},"▲")])]),(0,s._)("div",_,[(0,s._)("div",v,[(0,s._)("button",{onClick:o[7]||(o[7]=t=>D.moveAction("L")),class:"arrow-button arrow-left"},"▲")]),(0,s._)("div",f,[(0,s._)("button",{onClick:o[8]||(o[8]=t=>D.moveAction("R")),class:"arrow-button arrow-right"},"▲")])]),(0,s._)("div",w,[(0,s._)("div",y,[(0,s._)("button",{onClick:o[9]||(o[9]=t=>D.moveAction("D")),class:"arrow-button arrow-down"},"▲")])]),(0,s._)("button",{onClick:o[10]||(o[10]=t=>D.moveAction("S")),id:"stopBtn"},"STOP")])]),(0,s._)("div",C,[(0,s._)("div",S,[((0,s.wg)(!0),(0,s.iD)(s.HY,null,(0,s.Ko)(a.messages_for_message_box,(t=>((0,s.wg)(),(0,s.iD)("div",{class:"message",key:t},(0,r.zw)(t),1)))),128))])])])])}e(560);var P={name:"mainPage",data(){return{messages:[],socket:null,micro_conn:!1,messages_for_message_box:[],recording:!1,playback:!1,micro_state:0,state_message:"Manual Movement"}},props:{msg:String},methods:{moveAction(t){this.socket.send("[mv "+t+"]")},sendMessage(){this.socket.send("[hello]")},toggleLight(){this.socket.send("[toggle light]")},getDataStream(t){const o=/:\s*([\s\S]*)$/,e=t.data.match(o);if(e&&!(e.length<=1))return e[1].trim().split(/\s+/);console.log("invalid data parse")},recordInputs(){console.log("hit record"),this.socket.send("[record]")},initiateScan(){console.log("starting scan"),this.socket.send("[scan]")},stopRecordInputs(){console.log("hit stop record"),this.socket.send("[done rec]")},startPlayback(){console.log("start playback"),this.socket.send("[start play]"),this.socket.send("[playback]")},stopPlayback(){console.log("stop playback"),this.socket.send("[stp play]")}},mounted(){this.socket=new WebSocket("ws://174.129.215.96:3000"),this.socket.onopen=()=>{this.socket.send("[webpage]"),console.log("Connection msg sent"),this.socket.send("[micro_conn]"),console.log("Checking for micro connection msg sent"),this.socket.send("[checkMovementDetection]"),console.log("Checking for movement detection"),this.socket.send("[state]"),console.log("Checking micro state")},this.socket.onmessage=t=>{this.messages.push(t.data),console.log(t.data);const o=/\[([^\]]+)\]/,e=t.data.match(o);console.log(`result: ${e}`),(!e||e.length<=1)&&console.log("Error: could not parse [command] format");const n=e[1];switch(console.log(`command: ${n}`),n){case"detected":{const o=this.getDataStream(t);alert(`Motion Detected! [${o}]`),console.log(`Motion Detected! [${o}]`);var s="";for(const t of o)""!==t.trim()&&(s+=t.trim()+" ");this.messages_for_message_box.push(`Motion Detected! ${s}`);break}case"ledStatus":{const o=this.getDataStream(t);console.log(`LED Status: ${o}`);break}case"toggleLight":{const o=this.getDataStream(t);console.log(`LED Status: ${o}`);break}case"motor":{const o=this.getDataStream(t);console.log(`motor msg: ${o}`);break}case"micro_conn":this.micro_conn=1==parseInt(this.getDataStream(t)),console.log(`micro_conn received: ${this.micro_conn}`);break;case"state":switch(this.micro_state=parseInt(this.getDataStream(t)[0]),console.log(`micro_state received: ${this.micro_state}`),this.micro_state){case 0:this.state_message="Manual Movement";break;case 1:this.state_message="Scanning";break;case 2:this.state_message="Playback";break}break;case"movementDetection":{let o=t.data.substring(21);console.log(`movement: ${o}`),o=`[${o}]`;let e=o.replace(/,/g,'","');e=e.replace(/\[/g,'["'),e=e.replace(/\]/g,'"]');let n=JSON.parse(e);console.log(`movement detection received: ${n}`),n.forEach((t=>{""!=t&&this.messages_for_message_box.push(`Motion Detected at ${t}!`)}));break}case"p/r err":alert("Cannot playback and record simultaneously! Press stop on running process"),console.log("playback/recording error");break;case"recording status":console.log("received recording status"),this.recording=1==parseInt(this.getDataStream(t));break;case"playback status":console.log("received playback status"),this.playback=1==parseInt(this.getDataStream(t));break;default:console.log("Error: invalid command read")}}}},$=e(89);const M=(0,$.Z)(P,[["render",D]]);var O=M,x={name:"App",components:{mainPage:O}};const j=(0,$.Z)(x,[["render",a]]);var B=j;(0,n.ri)(B).mount("#app")}},o={};function e(n){var s=o[n];if(void 0!==s)return s.exports;var a=o[n]={exports:{}};return t[n].call(a.exports,a,a.exports,e),a.exports}e.m=t,function(){var t=[];e.O=function(o,n,s,a){if(!n){var r=1/0;for(u=0;u<t.length;u++){n=t[u][0],s=t[u][1],a=t[u][2];for(var c=!0,i=0;i<n.length;i++)(!1&a||r>=a)&&Object.keys(e.O).every((function(t){return e.O[t](n[i])}))?n.splice(i--,1):(c=!1,a<r&&(r=a));if(c){t.splice(u--,1);var l=s();void 0!==l&&(o=l)}}return o}a=a||0;for(var u=t.length;u>0&&t[u-1][2]>a;u--)t[u]=t[u-1];t[u]=[n,s,a]}}(),function(){e.n=function(t){var o=t&&t.__esModule?function(){return t["default"]}:function(){return t};return e.d(o,{a:o}),o}}(),function(){e.d=function(t,o){for(var n in o)e.o(o,n)&&!e.o(t,n)&&Object.defineProperty(t,n,{enumerable:!0,get:o[n]})}}(),function(){e.g=function(){if("object"===typeof globalThis)return globalThis;try{return this||new Function("return this")()}catch(t){if("object"===typeof window)return window}}()}(),function(){e.o=function(t,o){return Object.prototype.hasOwnProperty.call(t,o)}}(),function(){var t={143:0};e.O.j=function(o){return 0===t[o]};var o=function(o,n){var s,a,r=n[0],c=n[1],i=n[2],l=0;if(r.some((function(o){return 0!==t[o]}))){for(s in c)e.o(c,s)&&(e.m[s]=c[s]);if(i)var u=i(e)}for(o&&o(n);l<r.length;l++)a=r[l],e.o(t,a)&&t[a]&&t[a][0](),t[a]=0;return e.O(u)},n=self["webpackChunkmouseui"]=self["webpackChunkmouseui"]||[];n.forEach(o.bind(null,0)),n.push=o.bind(null,n.push.bind(n))}();var n=e.O(void 0,[998],(function(){return e(3591)}));n=e.O(n)})();
//# sourceMappingURL=app.42712629.js.map