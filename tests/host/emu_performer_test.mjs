// SPDX-License-Identifier: GPL-3.0-only
// Real browser WASM: physical UI gestures, phase, flexible sections and audio.
import fs from 'node:fs';
import assert from 'node:assert/strict';
const bytes=fs.readFileSync(process.argv[2]||'build/emu/x0x.wasm');
const white=[0,2,4,6,7,9,11,12,14,16,18,19,21,23,25,26];
async function boot(){
  let mem;
  const clock={clock_time_get:(a,b,o)=>{new DataView(mem.buffer).setBigUint64(o,0n,true);return 0;}};
  const {instance}=await WebAssembly.instantiate(bytes,{wasi_snapshot_preview1:clock});
  const e=instance.exports;mem=e.memory;e._initialize();e.web_boot();
  function frames(n,collect=false){const samples=[];for(let d=0;d<n;d+=128){const k=Math.min(128,n-d);e.web_render(k);if(collect)samples.push(...new Float32Array(mem.buffer,e.web_out_l(),k));}return samples;}
  const wait=ms=>frames(Math.round(ms*44.1));
  const tap=b=>{e.web_buttons(1<<b);wait(40);e.web_buttons(0);wait(40);};
  const key=w=>{e.web_keys(1<<white[w]);wait(60);e.web_keys(0);wait(30);};
  const turn=(k,d)=>{e.web_enc(k,d);wait(80);};
  const until=(predicate,max=12000)=>{for(let t=0;t<max&&!predicate();t+=20)wait(20);assert(predicate(),'musical boundary not reached');};
  wait(300);return {e,frames,wait,tap,key,turn,until,mem};
}
{
  const {e,wait,tap,key,turn,until,mem}=await boot();
  tap(13);tap(10);tap(3);assert.equal(e.web_live_mode(),1,'mixer latches after release');
  tap(2);assert.equal(e.web_live_mode(),3,'automation latches');turn(5,2);
  until(()=>e.web_section(2)===3);key(4);
  assert(e.web_auto_value(2,0,0)>48,'four-bar grow joins bar-three phase');
  until(()=>e.web_section(2)===1);assert(e.web_auto_value(2,0,0)<55,'grow resets at square end');
  e.web_keys(1<<5);wait(60);e.web_keys(0);wait(30);turn(3,1);
  assert.equal(e.web_auto_value(2,0,0),-1,'manual knob cancels automation');
  tap(8);assert.equal(e.web_live_mode(),4);key(3);key(8);
  assert.equal(e.web_section(3),5);until(()=>e.web_section(2)===5);
  assert.equal(e.web_section(0),1,'extra fifth bar stays in groove');
  until(()=>e.web_section(0)===3);assert.equal(e.web_section(2),1,'break establishes new origin');
  key(5);key(7);assert.equal(e.web_section(3),3,'shrink current segment');
  until(()=>e.web_section(0)===5);assert.equal(e.web_section(2),1,'drop follows revised boundary');
  assert(new Uint16Array(mem.buffer,e.web_fb(),240*240).some(v=>v),'screen drawn');
  console.log('emu performer: latching, late-join phase/reset, manual takeover, +1/-1 bar and new segment origin PASS');
}
for(const w of [4,5,6,7,8]){
  const {e,wait,tap,frames}=await boot();tap(13);tap(10);wait(800);tap(0);
  e.web_keys(1<<white[w]);wait(80);
  const n=w>=6?Math.round(44100*30/125/(1<<(w-6))):8192;
  const samples=frames(w>=6?n*3:44100,true);
  assert(samples.every(Number.isFinite),'finite audio');
  const peak=Math.max(...samples.map(Math.abs));assert(peak<2.01,'bounded audio');
  if(w>=6){
    let error=0;for(let i=n+128;i<samples.length-n;i++)error=Math.max(error,Math.abs(samples[i]-samples[i+n]));
    assert(peak>0.001,'repeat contains captured sound');assert(error<0.0001,`repeat periodicity ${error}`);
    console.log(`emu repeat ${w===6?'1/8':w===7?'1/16':'1/32'}: ${n} frames, peak ${peak.toFixed(3)}, cycle error ${error.toExponential(2)} PASS`);
  }else if(w===4){assert(Math.max(...samples.slice(-2048).map(Math.abs))<0.003,'tape slows into silence');console.log('emu tape stop: finite, fades to silence PASS');}
  else {assert(samples.filter(v=>Math.abs(v)<0.00001).length>300,'chopper opens silence');console.log('emu chopper: rhythmic silence PASS');}
  e.web_keys(0);wait(150);assert.equal(e.web_section(0),1);tap(10);
}
