"""
How to run:
1. Ensure you have OpenCV and requests installed (During development, venv was used, you can create a venv using "python -m venv venv && source venv/bin/activate")
2. Install the required packages: "pip install opencv-python requests numpy"
3. Connect your ESP32 camera to the same network as your computer.
4. Run the script: "python main.py --ip <ESP32_IP_ADDRESS>"
"""

import cv2
import numpy as np
import requests
import time
from collections import deque
import argparse

# Configuration
ESP32_IP = "192.168.38.154"  # Default IP address of ESP32 camera
CAMERA_URL = f"http://{ESP32_IP}/stream"
CONTROL_URL = f"http://{ESP32_IP}/control"
FRAME_WIDTH = 640
FRAME_HEIGHT = 480
CENTER_X = FRAME_WIDTH // 2
THRESHOLD_DISTANCE = 50  # Distance from center to trigger turning
COMMAND_DELAY = 0.2  # Delay between commands to prevent overwhelming the ESP32

def send_command(command):
    """Send command to ESP32"""
    try:
        response = requests.post(CONTROL_URL, data=command, timeout=1)
        print(f"Sent command: {command}, Response: {response.text}")
        return True
    except Exception as e:
        print(f"Error sending command: {e}")
        return False

class ObjectTracker:
    def __init__(self):
        # Initialize object tracking parameters
        self.last_command_time = time.time()
        # Recent positions for smoothing (reduce jitter)
        self.positions = deque(maxlen=5)
        # Create a background subtractor
        self.bg_subtractor = cv2.createBackgroundSubtractorMOG2(history=100, varThreshold=50, detectShadows=False)
        
    def find_most_prominent_object(self, frame):
        """Find the most prominent moving object in the frame"""
        # Apply background subtraction
        fg_mask = self.bg_subtractor.apply(frame)
        
        # Apply some morphology to clean up the mask
        kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
        fg_mask = cv2.morphologyEx(fg_mask, cv2.MORPH_OPEN, kernel)
        fg_mask = cv2.morphologyEx(fg_mask, cv2.MORPH_CLOSE, kernel)
        
        # Find contours in the mask
        contours, _ = cv2.findContours(fg_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        if not contours:
            return None, frame, fg_mask
            
        # Find the largest contour by area
        largest_contour = max(contours, key=cv2.contourArea, default=None)
        
        if largest_contour is None or cv2.contourArea(largest_contour) < 500:  # Minimum area threshold
            return None, frame, fg_mask
            
        # Get bounding box of the contour
        x, y, w, h = cv2.boundingRect(largest_contour)
        center_x = x + w // 2
        center_y = y + h // 2
        
        # Add position to smooth the tracking
        self.positions.append((center_x, center_y))
        
        # Calculate average position from recent positions
        if len(self.positions) > 0:
            avg_x = int(sum(pos[0] for pos in self.positions) / len(self.positions))
            avg_y = int(sum(pos[1] for pos in self.positions) / len(self.positions))
        else:
            avg_x, avg_y = center_x, center_y
        
        # Draw the bounding box and center on the frame
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.circle(frame, (avg_x, avg_y), 5, (0, 0, 255), -1)
        cv2.line(frame, (CENTER_X, 0), (CENTER_X, FRAME_HEIGHT), (255, 0, 0), 1)
        
        return (avg_x, avg_y), frame, fg_mask
    
    def process_frame(self, frame):
        """Process a frame and send appropriate commands"""
        # Resize frame for consistent processing
        frame = cv2.resize(frame, (FRAME_WIDTH, FRAME_HEIGHT))
        
        # Find most prominent object
        object_center, processed_frame, mask = self.find_most_prominent_object(frame)
        
        # If no object is detected, stop movement
        if object_center is None:
            if time.time() - self.last_command_time > COMMAND_DELAY:
                send_command("stop")
                self.last_command_time = time.time()
            return processed_frame, mask
        
        # Get object position
        obj_x, obj_y = object_center
        
        # Determine if we need to turn to track the object
        current_time = time.time()
        if current_time - self.last_command_time > COMMAND_DELAY:
            if obj_x < CENTER_X - THRESHOLD_DISTANCE:
                send_command("left")
                print("Turning left")
            elif obj_x > CENTER_X + THRESHOLD_DISTANCE:
                send_command("right")
                print("Turning right")
            else:
                send_command("stop")
                print("Centered - stopping")
            self.last_command_time = current_time
            
        return processed_frame, mask

def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="ESP32 Camera Object Tracker")
    parser.add_argument("--ip", default="192.168.38.154", help="ESP32 IP address")
    parser.add_argument("--video", help="Path to video file (for testing)")
    args = parser.parse_args()
    
    global ESP32_IP, CAMERA_URL, CONTROL_URL
    if args.ip:
        ESP32_IP = args.ip
        CAMERA_URL = f"http://{ESP32_IP}:81/stream"
        CONTROL_URL = f"http://{ESP32_IP}/control"
    
    # Create object tracker
    tracker = ObjectTracker()
    
    # Create a video capture object
    if args.video:
        # Use local video file for testing
        cap = cv2.VideoCapture(args.video)
    else:
        # Use ESP32 camera stream
        # Note: OpenCV's VideoCapture may not work with ESP32 stream directly
        # For a production system, consider using a more robust streaming method
        cap = cv2.VideoCapture(CAMERA_URL)
    
    if not cap.isOpened():
        print("Failed to open video stream or file")
        return
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to read frame")
            break
            
        # Process the frame for object tracking
        processed_frame, mask = tracker.process_frame(frame)
        
        # Display the processed frame
        cv2.imshow("Object Tracking", processed_frame)
        cv2.imshow("Mask", mask)
        
        # Break the loop if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            send_command("stop")
            break
    
    # Clean up
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
