# PD-01RGB-WIFI1 

This firmware is developed to be used in combination with IoT RGB LED Controller v2, which can be ordered from my Tindie store.
<img src="https://user-images.githubusercontent.com/34834753/154815356-172ba36e-41f8-4dd3-b6ec-6437522f118e.jpg" width="500">
<a href="https://www.tindie.com/stores/jonathancaes/?ref=offsite_badges&utm_source=sellers_JonathanCaes&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

## Latest release
<a href="https://github.com/JC-Electronics-Design/PD-01RGB-WIFI1/releases/tag/v2.1">Current stable version of the software is v2.1.</a>

## Setup
In case you bought the controller from my Tindie store the latest software is already preloaded onto the board. The only thing you have to do is connect your LED strip, connect the touch button (if your using it) and plug in the power cable. Below is an image with some indications on what is what. 

<img src="https://user-images.githubusercontent.com/34834753/154841525-c9ffe404-0397-4101-a4a5-aee859897f2b.jpg" width="750">

### Acces the Config Portal
Once you plug in power to the PCB, the ESP8266 will start in AP (Access Point) mode since there are no WiFi settings available yet. 
The ESP8266 will come up in your WiFi settings with the following SSID: `PD-01RGB-WIFI1`.
Once you connect to this AP it will ask for a password. The default password for this network is: `JCDesign`.

<img src="https://user-images.githubusercontent.com/34834753/154843146-eecc691c-ca86-44ef-8dca-bef05d211209.jpg" height="200"><img src="https://user-images.githubusercontent.com/34834753/154844131-e8918d5d-4cb4-42f1-aad4-73d1c25d500a.jpg" height="200">

When you are connected to the AP, the config portal will come up automatically. If the config portal does not show up by itself you can also browse to the following ip address: `192.168.4.1`. The window below will be called root window in further documentation. 

<img src="https://user-images.githubusercontent.com/34834753/154845040-71627d53-f507-4e9b-9150-5a361b400074.png" height="350"><img src="https://user-images.githubusercontent.com/34834753/154845044-1dfc9867-16b1-49ca-859c-7c0924e30e99.png" height="350">

#### Tip: Change `Setup` parameters before configuring WiFi. 
When configuring your WiFi settings the config portal will close after a successful connection to the network. Therefor it is best to first configure the Setup parameters. 

### Config Parameters
To configure the setup/device parameters click on `Setup` in the config portal. You will be greeted with the following window.

<img src="https://user-images.githubusercontent.com/34834753/154845447-8ca7edd1-c0db-4827-b884-2cfd29c0ca70.png" height="325"><img src="https://user-images.githubusercontent.com/34834753/154845459-f95c9adf-0ffb-49b0-837b-bc75a43cf482.png" height="325">

The first three parameters are to configure your MQTT server ip, port and MQTT device id. The following two parameters are the onboard temperature sensor MQTT topic and time between temperature readings. After that you can check the box whether you'd like to use the touch button or not and set the MQTT topic after that. The last parameter is the MQTT topic for the RGB LED strip output. Once all settings are according your needs press the `Save` button to store the values and go back to the root window. 

### WiFi Configuration
The last step in the config portal is to configure your WiFi network. Click `Configure WiFi` on the root page. It might take a few seconds to load the page. This is because the ESP8266 is looking for all networks that are in close proximity. In the end the following page will show up.

<img src="https://user-images.githubusercontent.com/34834753/154846042-98350ec9-e6e5-42e3-b4e9-9255ae28c8aa.png" height="325"><img src="https://user-images.githubusercontent.com/34834753/154846046-2648abc3-74cb-4c8e-84ff-ed0029e89e1e.png" height="325">

Here click on the network SSID you want the ESP8266 to connect to. This will copy the SSID to the SSID parameter below. After that fill in the password for this network. At last configure the static ip, static gateway ip and subnet mask you want to use. Now click the `Save` button to save the parameters and start connecting to the WiFi network. Once the ESP8266 is able to connect to the WiFi network the config portal will be closed and the device is ready to be used. Bare in mind that closing the config portal may take a few seconds. 

## FYI
- If you want to use the software on a selfmade PCB or for something else this can also be done. Just keep in mind that you might have to remap the pins to what you are using for your design. 

- PD-01RGB-WIFI1 is the internal reference I use for this product.
