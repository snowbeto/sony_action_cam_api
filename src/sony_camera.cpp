#include <sony_camera.hpp>


size_t read_func(void *ptr, size_t size, size_t count, void *data_container) {
	//add stream to buffer
	SACamera_Data *data = (SACamera_Data *) data_container;
	data->buffer.append((char*) ptr, size * count);

	//search for the start and end of the image in the buffer
	std::size_t found_start = data->buffer.find("\xff\xd8");
	std::size_t found_end = data->buffer.find("\xff\xd9");

	//there is an image in the stream
	if ((found_start != std::string::npos) && found_end != std::string::npos) {
		//extract image from buffer
		size_t end_pos = (found_end - found_start) + 2; //include end
		std::string image = data->buffer.substr(found_start, end_pos);

		//converti image to uint8_t array
		std::vector<uint8_t> myVector(image.begin(), image.end());

		//convert image to cv::Mat from uint8_t vector
		cv::Mat decodedImage = cv::imdecode(myVector, CV_LOAD_IMAGE_COLOR);

		if (decodedImage.data != NULL) {
			//data->images.push(decodedImage);
			data->image = decodedImage;
		} else {
			// Error reading raw image data
			cout << "Could not read image" << endl;
			//data->buffer.clear();
			//data->buffer.reserve(1000000);
		}

		//remove read image from buffer
		data->buffer.erase(found_start, end_pos);

	}

	//std::cout << data->buffer.size() << std::endl;
	return count * size;
}
;

int progress_func(void *data_container, double dltotal, double dlnow,
		double ultotal, double ulnow) {
	SACamera_Data *data = (SACamera_Data *) data_container;
	if (data->read)
		return 0;
	return 1;
}
;

curl_easy& SACamera::get_livestream_handler() {
	return livestream_handler;
}
;

bool SACamera::can_read_image() {
	if (data.image.data != NULL)
		return true;

	return false;
}
;
cv::Mat SACamera::get_image() {
	return data.image.clone();
}

std::string SACamera::generate_command(CameraCommand cmd) {
	std::string str_cmd;
	std::string mode;
	std::string params;

	switch (cmd) {
	case START_RECORDING:
		mode = "\"startMovieRec\"";
		params = "";
		break;
	case STOP_RECORDING:
		mode = "\"stopMovieRec\"";
		params = "";
		break;
	case SET_CAMERA_MODE_MOVIE:
		mode = "\"setShootMode\"";
		params = "\"movie\"";
		break;
	case START_LIVEVIEW:
		mode = "\"startLiveview\"";
		params = "";
		break;
	case TAKE_PICTURE:
		mode = "\"actTakePicture\"";
		params = "";
		break;
	}
	str_cmd = "{\"method\" : " + mode + ", \"params\" : [" + params
			+ "], \"id\" : " + std::to_string(id++)
			+ ", \"version\" : \"1.0\" }";

	return str_cmd;
}

void SACamera::send_command(CameraCommand cmd) {
	curl_slist *headers = NULL;
	curl_slist_append(headers, "Content-Type: application/json");
	std::string request = generate_command(cmd);
	// Add some option to the easy handle
	std::ostringstream str;
	// Create a curl_ios object, passing the stream object.
	curl_ios<std::ostringstream> writer(str);
	curl_easy camera_handler;
	camera_handler.add<CURLOPT_URL>(camera_url.c_str());
	camera_handler.add<CURLOPT_HTTPHEADER>(headers);
	camera_handler.add<CURLOPT_CUSTOMREQUEST>("POST");
	camera_handler.add<CURLOPT_POSTFIELDS>(request.c_str());
	camera_handler.perform();
	//cout << "Request: " << request << endl;;
	//cout<< "Respose:" << str.str()<<endl;

}
;

void SACamera::clear_buffer() {
	data.buffer.clear();
	data.buffer.reserve(50000);
}

SACamera::SACamera(std::string ip_address, std::string port) :
		ip_address(ip_address), port(port), id(1) {
	liveviewstream_url = "http://" + ip_address + ":" + port
			+ "/liveview/liveviewstream";
	camera_url = "http://" + ip_address + ":" + port + "/sony/camera";

	//Initialze livestream_handler
	livestream_handler.add<CURLOPT_URL>(liveviewstream_url.c_str());
	livestream_handler.add<CURLOPT_WRITEFUNCTION>(read_func);
	livestream_handler.add< CURLOPT_WRITEDATA>(&data);
	livestream_handler.add<CURLOPT_PROGRESSFUNCTION>(progress_func);
	livestream_handler.add<CURLOPT_PROGRESSDATA>(&data);

}
;

SAMultiCameraHandler::SAMultiCameraHandler(SACamera& camera1, SACamera& camera2) :
		camera1(camera1), camera2(camera2) {
	handler.add(camera1.get_livestream_handler());
	handler.add(camera2.get_livestream_handler());
}

void SAMultiCameraHandler::read_streams() {
	handler.perform();
}

void SAMultiCameraHandler::read_camera_individual_stream_1() {
	camera1.get_livestream_handler().perform();
}
void SAMultiCameraHandler::read_camera_individual_stream_2() {
	camera2.get_livestream_handler().perform();
}
void SAMultiCameraHandler::read() {
	camera_thread = std::thread(&SAMultiCameraHandler::read_streams, this);
	camera_thread.join();
}

void SAMultiCameraHandler::read_individual() {
	camera_1_thread = std::thread(
			&SAMultiCameraHandler::read_camera_individual_stream_1, this);
	camera_2_thread = std::thread(
			&SAMultiCameraHandler::read_camera_individual_stream_2, this);

}

void SAMultiCameraHandler::send_command(SACamera * camera,
		SACamera::CameraCommand cmd) {
	camera->send_command(cmd);
}

void SAMultiCameraHandler::start_recording() {
	std::thread t_camera1(&SAMultiCameraHandler::send_command, this, &camera1,
			SACamera::START_RECORDING);
	std::thread t_camera2(&SAMultiCameraHandler::send_command, this, &camera2,
			SACamera::START_RECORDING);

	t_camera1.detach();
	t_camera2.detach();
}

void SAMultiCameraHandler::take_images() {
	std::thread t_camera1(&SAMultiCameraHandler::send_command, this, &camera1,
			SACamera::TAKE_PICTURE);
	std::thread t_camera2(&SAMultiCameraHandler::send_command, this, &camera2,
			SACamera::TAKE_PICTURE);

	t_camera1.detach();
	t_camera2.detach();
}

void SAMultiCameraHandler::stop_recording() {
	std::thread t_camera1(&SAMultiCameraHandler::send_command, this, &camera1,
			SACamera::STOP_RECORDING);
	std::thread t_camera2(&SAMultiCameraHandler::send_command, this, &camera2,
			SACamera::STOP_RECORDING);

	t_camera1.detach();
	t_camera2.detach();
}

