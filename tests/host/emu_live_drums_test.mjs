// SPDX-License-Identifier: GPL-3.0-only
// Real UI and WASM audio: synchronized effects, independent rolls and routing.
import fs from 'node:fs';
import assert from 'node:assert/strict';
const bytes=fs.readFileSync(process.argv[2]||'build/emu/x0x.wasm');
const white=[0,2,4,6,7,9,11,12,14,16,18,19,21,23,25,26];
async function boot(){
 let mem;
 const {instance}=await WebAssembly.instantiate(bytes,{wasi_snapshot_preview1:{clock_time_get:(a,b,o)=>{new DataView(mem.buffer).setBigUint64(o,0n,true);return 0;}}});
 const e=instance.exports;mem=e.memory;e._initialize();e.web_boot();
 const frames=(n,collect=false)=>{const out=[];for(let j=0;j<n;j+=128){const k=Math.min(128,n-j);e.web_render(k);if(collect)out.push(...new Float32Array(mem.buffer,e.web_out_l(),k));}return out;};
 const wait=ms=>frames(Math.round(ms*44.1));
 const tap=b=>{e.web_buttons(1<<b);wait(40);e.web_buttons(0);wait(40);};
 const key=k=>{e.web_keys(1<<k);wait(40);e.web_keys(0);wait(40);};
 const turn=(k,d)=>{e.web_enc(k,d);wait(32);};
 const until=f=>{for(let n=0;n<44100*3&&!f();n+=64)frames(64);assert(f(),'event deadline');};
 const grid=()=>{const step=e.web_grid_step();until(()=>e.web_grid_step()!==step);};
 wait(300);return {e,frames,wait,tap,key,turn,until,grid};
}
{
 const {e,key,turn,tap}=await boot();
 key(5);assert.equal(e.web_live_extra(),1);key(5);assert.equal(e.web_live_extra(),0);
 key(5);turn(5,-127);assert.equal(e.web_drive_position(2),0);
 for(let v=0;v<128;v++){
   if(v)turn(5,1);
   const rank=v-63,amount=!v?0:v<64?1+Math.floor((v-1)*126/62):rank<=5?rank:6+Math.floor((rank-6)*121/58);
   assert.equal(e.web_drive_position(2),v,'combined drive inverse');
   assert.equal(e.web_native_sound(2,0,8),amount,'native drive amount');
   assert.equal(e.web_native_sound(2,0,9),!v?0:v<64?1:2,'native model');
 }
 turn(6,-200);assert.equal(e.web_transpose(0),-24);turn(6,200);assert.equal(e.web_transpose(0),24);
 tap(0);tap(0);tap(0);tap(0);assert.equal(e.web_fx_view(),3);
 const attack=e.web_native_sound(6,0,2);turn(4,2);assert(e.web_native_sound(6,0,2)>attack);
 tap(0);assert.equal(e.web_fx_view(),4);
 for(let n=0;n<10;n++)turn(4,-1);
 assert.equal(e.web_native_sound(6,0,1),0);
 console.log('live controls: 128 Drive positions/RAT toe, transpose bounds, repeated selection and compressor tuning PASS');
}
for(let fx=0;fx<9;fx++){
 const {e,tap,wait,frames,grid,until}=await boot();tap(13);tap(10);tap(0);grid();wait(25);
 e.web_keys(1<<white[fx]);wait(20);
 assert.equal(e.web_momentary(0),-1,'no immediate onset');assert.equal(e.web_momentary(1),fx);
 const before=e.web_grid_step();until(()=>e.web_momentary(0)===fx);
 assert.notEqual(e.web_grid_step(),before,'front is on a sequencer 16th');
 const audio=frames(4096,true);assert(audio.every(Number.isFinite));
 e.web_keys(0);wait(30);assert.equal(e.web_momentary(0),-1);
 console.log(`quantized effect ${fx}: waits for grid, finite audio, release PASS`);
}
{
 const {e,tap,wait,grid}=await boot();tap(13);tap(10);tap(0);grid();wait(25);
 e.web_keys(1<<white[5]);wait(20);e.web_keys(0);wait(20);grid();
 assert.equal(e.web_momentary(0),-1);assert.equal(e.web_momentary(1),-1);
 e.web_keys(1<<white[5]);grid();wait(8);assert(e.web_chop_phase()<0.15,'open edge at attack');
 wait(60);assert(e.web_chop_phase()>0.5,'closed tail half');
 grid();wait(8);assert(e.web_chop_phase()<0.15,'phase relocks on next attack');
 console.log('chopper: early release cancels pending onset; open attack / closed tail grid phase PASS');
}
for(let rate=0;rate<3;rate++){
 const {e,tap,wait,grid,until,frames}=await boot();tap(13);tap(10);tap(0);grid();wait(25);
 e.web_keys(1<<white[9+rate]);wait(20);assert.equal(e.web_roll(2),0,'roll waits for grid');
 until(()=>e.web_roll(0)===1);const first=e.web_roll(2);
 for(let n=0;n<8;n++)grid();
 assert.equal(e.web_roll(2)-first,[4,8,16][rate],'roll rate over eight 16ths');
 e.web_keys((1<<white[9+rate])|(1<<white[0]));until(()=>e.web_momentary(0)===0);
 const count=e.web_roll(2);wait(270);assert(e.web_roll(2)>count,'LP and roll run together');
 e.web_enc(3,4);wait(40);assert.equal(e.web_roll(0),1,'filter knob retains roll');
 const audio=frames(8192,true);assert(audio.every(Number.isFinite));assert(Math.max(...audio.map(Math.abs))<2.01);
 e.web_keys(1<<white[0]);wait(25);const stopped=e.web_roll(2);wait(300);assert.equal(e.web_roll(2),stopped);
 e.web_keys(0);wait(25);assert.equal(e.web_momentary(0),-1);
 e.web_keys(1<<white[9+rate]);grid();tap(10);wait(40);assert.equal(e.web_roll(0),0);
 console.log(`snare roll ${rate}: grid/rate, concurrent LP, knob, key release and transport stop PASS`);
}
async function isolated(filter,bass){
 const {e,tap,key,wait,frames,grid}=await boot();tap(13);
 if(!bass){key(1);tap(13);}
 e.web_buttons(1<<11);e.web_keys(1<<(bass?5:1));wait(40);e.web_keys(0);e.web_buttons(0);wait(40);
 tap(10);tap(0);grid();wait(25);e.web_keys(filter?1:1<<white[15]);wait(200);
 return frames(12000,true);
}
{
 const {e,tap,key,wait,grid,until}=await boot();key(3);tap(13);tap(10);tap(0);
 e.web_keys(1<<white[10]);until(()=>e.web_roll(0)===1);assert.equal(e.web_roll(3),1,'808 roll target');
 const muteKit=()=>{e.web_buttons(1<<6);wait(30);e.web_keys((1<<white[10])|(1<<3));wait(30);e.web_keys(1<<white[10]);wait(30);e.web_buttons(0);wait(30);};
 muteKit();const silent=e.web_roll(2);for(let n=0;n<4;n++)grid();assert.equal(e.web_roll(2),silent,'kit mute wins');
 muteKit();const resume=e.web_roll(2);grid();assert(e.web_roll(2)>resume,'unmute resumes held roll');
 e.web_keys(0);wait(30);assert.equal(e.web_roll(1),0);
 e.web_keys(1<<white[11]);wait(20);e.web_keys(0);wait(20);const cancel=e.web_roll(2);grid();assert.equal(e.web_roll(2),cancel,'pending roll cancellation');
 console.log('808 roll: target selection, whole-kit mute/unmute, release and pending cancellation PASS');
}
for(const bass of [true,false]){
 const a=await isolated(false,bass),b=await isolated(true,bass);
 let diff=0,peak=0;for(let n=0;n<a.length;n++){diff=Math.max(diff,Math.abs(a[n]-b[n]));peak=Math.max(peak,Math.abs(a[n]));}
 assert(peak>0.001,'audible reference');if(bass)assert.equal(diff,0,'303 is bit-identical through drum-only filter');else assert(diff>0.001,'drums are filtered');
 console.log(`drum filter routing: ${bass?'isolated 303 unchanged':'isolated drums changed'} PASS`);
}
{
 const {e,tap,turn,wait,grid,until}=await boot();
 tap(7);wait(3000); // persist default filter 64
 tap(13);tap(10);tap(0);turn(3,-10);
 assert.equal(e.web_native_sound(6,0,13),54);
 e.web_keys(1<<white[0]);until(()=>e.web_momentary(0)===0);
 e.web_test_restored();wait(100);e.web_keys(0);wait(40);
 assert.equal(e.web_native_sound(6,0,13),64,'saved pot restored');
 assert.equal(e.web_master_value(13),64,'old active snapshot cannot overwrite restored engine pot');
 assert.equal(e.web_momentary(0),-1);assert.equal(e.web_momentary(1),-1);assert.equal(e.web_roll(1),0);
 tap(0);assert.equal(e.web_live_mode(),2);
 console.log('project restore: pending/active gestures retired before loaded pots PASS');
}
