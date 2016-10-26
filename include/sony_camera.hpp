/*
 * sony_camera.hpp
 *
 *  Created on: Jul 25, 2016
 *      Author: daniel
 */

#ifndef INCLUDE_SONY_CAMERA_HPP_
#define INCLUDE_SONY_CAMERA_HPP_

#include <iostream>

#include <string>
#include <queue>
#include "curl_easy.h"
#include "curl_multi.h"
#include "curl_ios.h"
#include <ostream>
#include <thread>
#include <chrono>

#include <stdio.h>
#include <curlpp/Easy.hpp>


using std::cout;
using std::endl;

using curl::curl_easy;
using curl::curl_multi;
using curl::curl_ios;

class SACamera_Data {
public:
	std::string buffer;
	std::vector<uint8_t> image;
	bool read;

	SACamera_Data() :
			read(true) {
		buffer.reserve(1000000);
	}
	;
};

class SACamera {
public:

	enum CameraCommand {
		START_RECORDING,
		STOP_RECORDING,
		SET_CAMERA_MODE_MOVIE,
		START_LIVEVIEW,
		TAKE_PICTURE
	};

	SACamera_Data data;

	curl_easy& get_livestream_handler();

	bool can_read_image();
	std::vector<uint8_t> get_image();

	std::string generate_command(CameraCommand cmd);

	void send_command(CameraCommand cmd);
	;

	void clear_buffer();

	SACamera(std::string ip_address, std::string port);

private:
	std::string ip_address;
	std::string port;
	std::string liveviewstream_url;
	std::string camera_url;
	curl_easy livestream_handler;
	int id;

};

class SAMultiCameraHandler {
private:
	curl_multi handler;
	std::thread camera_thread;
	SACamera& camera1;
	SACamera& camera2;

public:
	std::thread camera_1_thread;
	std::thread camera_2_thread;
	SAMultiCameraHandler(SACamera& camera1, SACamera& camera2);

	void read_streams();
	void read_camera_individual_stream_1();
	void read_camera_individual_stream_2();
	void read();
	void read_individual();
	void send_command(SACamera * camera, SACamera::CameraCommand cmd);
	void start_recording();
	void stop_recording();
	void take_images();

};

#endif /* INCLUDE_SONY_CAMERA_HPP_ */
