import processing.sound.*;
import oscP5.*;
import netP5.*;

int channels = 3;                // number of channels to read
AudioIn[] ins;
Amplitude[] amps;
float[] smoothedLevels;
float smoothing = 0.1;           // smaller = more responsive

OscP5 osc;
NetAddress broadcast;
String[] paths = {"/amp/ch1", "/amp/ch2", "/amp/ch3"};
String deviceName = "BlackHole 16ch"; // exact name of your input device

void setup() {
  size(400, 200);
  
  osc = new OscP5(this, 12000);
  broadcast = new NetAddress("192.168.1.255", 9000);  // subnet broadcast
  
  ins = new AudioIn[channels];
  amps = new Amplitude[channels];
  smoothedLevels = new float[channels];
  
  for (int i = 0; i < channels; i++) {
    try {
      ins[i] = new AudioIn(this, i, deviceName);
      ins[i].start();
      
      amps[i] = new Amplitude(this);
      amps[i].input(ins[i]);
      
      smoothedLevels[i] = 0;
      println("Channel " + i + " started successfully.");
    } catch (Exception e) {
      println("Channel " + i + " failed: " + e.getMessage());
    }
  }
}

void draw() {
  background(0);
  
  for (int ch = 0; ch < channels; ch++) {
    float level = 0;
    try {
      level = amps[ch].analyze();       // RMS amplitude
      smoothedLevels[ch] = smoothing * level + (1 - smoothing) * smoothedLevels[ch];
      
      OscMessage msg = new OscMessage(paths[ch]);
      msg.add(smoothedLevels[ch]);
      osc.send(msg, broadcast);
      
      fill(0, 200, 0);
      rect(50 + ch*100, height, 50, -smoothedLevels[ch] * 150);
    } catch (Exception e) {
      // skip channels that failed
    }
  }
}
