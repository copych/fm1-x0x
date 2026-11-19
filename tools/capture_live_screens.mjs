// SPDX-License-Identifier: GPL-3.0-only
// Documentation captures from the real WASM framebuffer; no test assertions.
import fs from 'node:fs';
import path from 'node:path';
import zlib from 'node:zlib';
const bytes=fs.readFileSync('build/emu/x0x.wasm'),output='docs/img/performer';
fs.mkdirSync(output,{recursive:true});
function crc32(bytes){let c=0xffffffff;for(const b of bytes){c^=b;for(let n=0;n<8;n++)c=(c>>>1)^((c&1)?0xedb88320:0);}return (c^0xffffffff)>>>0;}
function chunk(name,data){const type=Buffer.from(name),out=Buffer.alloc(data.length+12);out.writeUInt32BE(data.length);type.copy(out,4);data.copy(out,8);out.writeUInt32BE(crc32(Buffer.concat([type,data])),data.length+8);return out;}
function png(pixels){
  const raw=Buffer.alloc(240*(1+240*3));
  for(let y=0;y<240;y++)for(let x=0;x<240;x++){
    const v=pixels[y*240+x],o=y*721+1+x*3;
    raw[o]=Math.round((v>>>11)*255/31);raw[o+1]=Math.round(((v>>>5)&63)*255/63);raw[o+2]=Math.round((v&31)*255/31);
  }
  const header=Buffer.alloc(13);header.writeUInt32BE(240,0);header.writeUInt32BE(240,4);header[8]=8;header[9]=2;
  return Buffer.concat([Buffer.from([137,80,78,71,13,10,26,10]),chunk('IHDR',header),chunk('IDAT',zlib.deflateSync(raw)),chunk('IEND',Buffer.alloc(0))]);
}
async function capture(name,actions){
  let mem;
  const {instance}=await WebAssembly.instantiate(bytes,{wasi_snapshot_preview1:{clock_time_get:(a,b,o)=>{new DataView(mem.buffer).setBigUint64(o,0n,true);return 0;}}});
  const e=instance.exports;mem=e.memory;e._initialize();e.web_boot();
  const wait=ms=>{for(let n=0;n<Math.round(ms*44.1);n+=128)e.web_render(128);};
  const tap=b=>{e.web_buttons(1<<b);wait(40);e.web_buttons(0);wait(60);};
  const key=k=>{e.web_keys(1<<k);wait(40);e.web_keys(0);wait(60);};
  const turn=(k,d)=>{e.web_enc(k,d);wait(80);};
  wait(400);actions({tap,key,turn});wait(2200);
  fs.writeFileSync(path.join(output,name+'.png'),png(new Uint16Array(mem.buffer,e.web_fb(),240*240)));
  console.log('Captured '+name);
}
await capture('live',()=>{});
await capture('drums',({key})=>{key(1);key(2);});
await capture('mix',({tap})=>tap(3));
await capture('pan',({tap})=>{tap(3);tap(3);});
await capture('automation',({tap,turn})=>{tap(2);turn(5,2);});
await capture('director',({tap})=>tap(8));
await capture('fx',({tap})=>tap(0));
await capture('delay',({tap})=>{tap(0);tap(0);});
await capture('reverb',({tap})=>{tap(0);tap(0);tap(0);});
