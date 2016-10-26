#include <sony_camera.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

void get_live_streams(SACamera& camera_left, SACamera& camera_right,
		SAMultiCameraHandler &handler) {
	std::string windowName1 = "Left";
	std::string windowName2 = "Right";

	while (true) {
		
		handler.read();
		cv::Mat left, right;
		if (camera_left.can_read_image() && camera_right.can_read_image()) {
			left = cv::imdecode(camera_left.get_image(), CV_LOAD_IMAGE_COLOR);
			right = cv::imdecode(camera_right.get_image(),CV_LOAD_IMAGE_COLOR);

			cv::imshow(windowName1, left);
			cv::imshow(windowName2, right);

			int keyPressed = cv::waitKey(1);

			if (keyPressed == 1048603) {		
					camera_left.data.read = false;
					camera_right.data.read = false;
					break;
		
			}

		}
	}
	
	cv::destroyWindow(windowName1);
	cv::destroyWindow(windowName2);
}

int main()
{
	SACamera camera_left("192.168.0.102","8080"), camera_right(
			"192.168.0.101","8080");
	SAMultiCameraHandler handler(camera_left, camera_right);
	get_live_streams(camera_left,  camera_right, handler);
	
}
