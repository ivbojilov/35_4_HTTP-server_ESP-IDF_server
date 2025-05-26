ESP-IDF template app
====================

54cb76b - the client sends "Hello"; the server prints it; works  
869cb44 - sending the int '10'; the client sends it as a string, the server parses the string to int; corresponds to clientside 67533f4  
9172980 - client converts an int to string and then sends it; corresponds to clientside 88be0d2  
3312f34 - sending 2 numbers; using int arrays in client and server; client creates a string from 2 ints, server parses the 2 numbers back into ints; corresponds to clientside f6b33d7  
881ecd0 - client sends 400 uints every 2 seconds; server receives well and prints the full body; clientside 8d6052a    
&nbsp;  -  
&nbsp;  -  
&nbsp;  -  
&nbsp;  -  
&nbsp;  -  
&nbsp;  -  
&nbsp;  -  
&nbsp;  -  
This is a template application to be used with [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

Please check [ESP-IDF docs](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for getting started instructions.

*Code in this repository is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.*
