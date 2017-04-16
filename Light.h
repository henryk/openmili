class Lamp{
public:

static uint32_t toRgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

static double mapIntensity(byte intensity){
    if (intensity >= 0 && intensity <= 0x90){
       return (double)(0x90 - intensity) / 0xE8;
    }else if (intensity >= 0xA8 && intensity <= 0xFF){
       return  (double)(0x98 + 0xF8 - intensity) / 0xE8;
    }
}

static uint32_t milightColor(byte wheelPos, byte intensity) {
    double scale = mapIntensity(intensity);
        
    wheelPos = 255 - (((uint32_t)wheelPos + 0x0000FF - 0x00001B) % 256);
    
    if (wheelPos < 85) {
        return toRgb((255 - wheelPos * 3) * scale, 0, (wheelPos * 3) * scale);
    }
    if (wheelPos < 170) {
        wheelPos -= 85;
        return toRgb(0, (wheelPos * 3) * scale, (255 - wheelPos * 3) * scale);
    }
    wheelPos -= 170;
    return toRgb((wheelPos * 3) * scale, (255 - wheelPos * 3) * scale, 0);
}

static uint32_t milightGray(byte intensity){
	double scale = mapIntensity(intensity);
	return toRgb(255 * scale, 255 * scale, 255 * scale);
}

private:

static bool isChannelButton(byte button){
	return button >= 1 && button <= 10;
}

byte getChannelOnButton() const{
	return GLOBAL_ON_BUTTON + myChannel * 2;
}

byte getChannelOffButton() const{
	return GLOBAL_OFF_BUTTON + myChannel * 2;
}

public:
void parse(byte color, byte intensity, byte cmd){  
    byte button = cmd & 0x0F;
    byte prolongedPress = (cmd & 0xF0) >> 4;
	
	if (  button == GLOBAL_ON_BUTTON
	   || button == GLOBAL_OFF_BUTTON 
	   || button == getChannelOnButton() 
	   || button == getChannelOffButton()
	){
		isTalkingToMe = true;
		isOn = button == GLOBAL_ON_BUTTON || button == getChannelOnButton();
	}else if (isChannelButton(button)){
		isTalkingToMe = false;
	}
	
    if (isTalkingToMe && isOn){
        if (button == WHEEL_BUTTON){
            inColorMode = true;
        }
        if (prolongedPress == 1){
            inColorMode = false;
        }
		
		if (inColorMode){
			myColor = milightColor(color, intensity);
		}else{
			myColor = milightGray(intensity);
		}
    }
}

uint32_t getColor(){
	return myColor;
}

bool isON(){
	return isOn;
}

Lamp(byte channel) : myChannel(channel){
	
}

private:
	bool isTalkingToMe = false;
	bool inColorMode = false;
	uint32_t myColor = 0xFFFFFF;
	bool isOn = false;
	byte myChannel = 2;
	
	static const byte GLOBAL_ON_BUTTON = 1;
	static const byte GLOBAL_OFF_BUTTON = 2;
	static const byte WHEEL_BUTTON = 15;
};
