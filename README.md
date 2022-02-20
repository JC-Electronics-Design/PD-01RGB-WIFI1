# PD-01RGB-WIFI1 

This firmware is developed to be used in combination with IoT RGB LED Controller v2, which can be ordered from my Tindie store.
<img src="https://user-images.githubusercontent.com/34834753/154815356-172ba36e-41f8-4dd3-b6ec-6437522f118e.jpg" width="500">
<a href="https://www.tindie.com/stores/jonathancaes/?ref=offsite_badges&utm_source=sellers_JonathanCaes&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

## Latest release
<a href="https://github.com/JC-Electronics-Design/PD-01RGB-WIFI1/releases/tag/v2.1">Current stable version of the software is v2.1.</a>

## Setup
In case you bought the controller from my Tindie store the software is already preloaded onto the board. The only thing you have to do is connect your LED strip, connect the touch button (if your using it) and plug in the power cable. Below is an image with some indications on what is what. 

<img src="https://user-images.githubusercontent.com/34834753/154841525-c9ffe404-0397-4101-a4a5-aee859897f2b.jpg" width="750">

### Acces Config Portal
Once you plug in power to the PCB, the ESP8266 will start in AP (Access Point) mode since there a no WiFi settings available yet. 
The ESP8266 will come up in your WiFi settings with the following SSID: "PD-01RGB-WIFI1".
Once you connect to this AP it will ask for a password. The default password for this network is: "JCDesign".

<img src="https://user-images.githubusercontent.com/34834753/154843146-eecc691c-ca86-44ef-8dca-bef05d211209.jpg" width="250">


### Config Parameters




## FYI
- If you want to use the software on a selfmade PCB or for something else this can also be done. Just keep in mind that you might have to remap the pins to what you are using for your design. 

- PD-01RGB-WIFI1 is the internal reference I use for this product.
