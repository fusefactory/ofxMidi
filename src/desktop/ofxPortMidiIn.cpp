#include "ofxPortMidiIn.h"

RtMidiIn ofxPortMidiIn::s_midiIn("ofxMidi Client");

// -----------------------------------------------------------------------------
ofxPortMidiIn::ofxPortMidiIn(const string name) :
	ofxBaseMidiIn(name), midiIn(name) {}

// -----------------------------------------------------------------------------
ofxPortMidiIn::~ofxPortMidiIn() {
	closePort();
}

// -----------------------------------------------------------------------------
// TODO: replace cout with ofLogNotice when OF_LOG_NOTICE is the default log level
void ofxPortMidiIn::listPorts() {
	cout << "ofxMidiIn: " << s_midiIn.getPortCount() << " ports available" << endl;
	for(unsigned int i = 0; i < s_midiIn.getPortCount(); ++i){
		cout << "ofxMidiIn: " <<  i << ": " << s_midiIn.getPortName(i) << endl;
	}
}

// -----------------------------------------------------------------------------
vector<string>& ofxPortMidiIn::getPortList() {
	portList.clear();
	for(unsigned int i=0; i < s_midiIn.getPortCount(); ++i) {
		portList.push_back(s_midiIn.getPortName(i));
	}
	return portList;
}

// -----------------------------------------------------------------------------
int ofxPortMidiIn::getNumPorts() {
	return s_midiIn.getPortCount();
}

// -----------------------------------------------------------------------------
string ofxPortMidiIn::getPortName(unsigned int portNumber) {
	// handle rtmidi exceptions
	try {
		return s_midiIn.getPortName(portNumber);
	}
	catch(RtError& err) {
		ofLog(OF_LOG_ERROR, "ofxMidiIn: couldn't get name for port %i: %s",
			portNumber, err.what());
	}
	return "";
}

// -----------------------------------------------------------------------------
bool ofxPortMidiIn::openPort(unsigned int portNumber) {	
	// handle rtmidi exceptions
	try {
		closePort();
		midiIn.setCallback(&_midiMessageCallback, this);
		midiIn.openPort(portNumber, "ofxMidi Input "+ofToString(portNumber));
	}
	catch(RtError& err) {
		ofLog(OF_LOG_ERROR, "ofxMidiIn: couldn't open port %i: %s", portNumber, err.what());
		return false;
	}
	portNum = portNumber;
	portName = midiIn.getPortName(portNumber);
	bOpen = true;
	ofLog(OF_LOG_VERBOSE, "ofxMidiIn: opened port %i %s",
		portNum, portName.c_str());
	return true;
}

// -----------------------------------------------------------------------------
bool ofxPortMidiIn::openPort(string deviceName) {
	
	// iterate through MIDI ports, find requested device
	int port = -1;
	for(unsigned int i = 0; i < midiIn.getPortCount(); ++i) {
		string name = midiIn.getPortName(i);
		if(name == deviceName) {
			port = i;
			break;
		}
	}
	
	// bail if not found
	if(port == -1) {
		ofLog(OF_LOG_ERROR, "ofxMidiIn: port \"%s\" is not available", deviceName.c_str());
		return false;
	} 
	
	return openPort(port);
}

// -----------------------------------------------------------------------------
bool ofxPortMidiIn::openVirtualPort(string portName) {
	// handle rtmidi exceptions
	try {
		closePort();
		midiIn.setCallback(&_midiMessageCallback, this);
		midiIn.openVirtualPort(portName);
	}
	catch(RtError& err) {
		ofLog(OF_LOG_ERROR, "ofxMidiIn: couldn't open virtual port \"%s\": %s",
			portName.c_str(), err.what());
		return false;
	}
	this->portName = portName;
	bOpen = true;
	bVirtual = true;
	ofLog(OF_LOG_VERBOSE, "ofxMidiIn: opened virtual port %s", portName.c_str());
	return true;
}

// -----------------------------------------------------------------------------
void ofxPortMidiIn::closePort() {
	if(bVirtual && bOpen) {
		ofLog(OF_LOG_VERBOSE, "ofxMidiIn: closing virtual port %s", portName.c_str());
	}
	else if(bOpen && portNum > -1) {
		ofLog(OF_LOG_VERBOSE, "ofxMidiIn: closing port %i %s", portNum, portName.c_str());
	}
	midiIn.closePort();
	if(bOpen)
		midiIn.cancelCallback();
	portNum = -1;
	portName = "";
	bOpen = false;
	bVirtual = false;
}

// -----------------------------------------------------------------------------
void ofxPortMidiIn::ignoreTypes(bool midiSysex, bool midiTiming, bool midiSense) {
	midiIn.ignoreTypes(midiSysex, midiTiming, midiSense);
	ofLog(OF_LOG_VERBOSE, "ofxMidiIn: ignore types on %s: sysex: %d timing: %d sense: %d",
			portName.c_str(), midiSysex, midiTiming, midiSense);
}
// -----------------------------------------------------------------------------
void ofxPortMidiIn::_midiMessageCallback(double deltatime, vector<unsigned char> *message, void *userData) {
	((ofxPortMidiIn*) userData)->manageNewMessage(deltatime * 1000, message); // convert s to ms
}