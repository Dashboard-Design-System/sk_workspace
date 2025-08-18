# app.py
from flask import Flask, request, jsonify, render_template

# Initialize the Flask application
app = Flask(__name__)

# A global dictionary to store the latest sensor data
# This acts as the central storage for our application.
sensor_data = {
    'temperature': 0.0,
    'speed': 0.0,
    'level': 0.0
}

# Route to receive data from the NodeMCU (ESP8266) via a POST request
@app.route('/sensors', methods=['POST'])
def receive_sensor_data():
    """Receives and updates sensor data from the hardware."""
    try:
        # Get the JSON data from the request body
        data = request.json
        if data:
            # Update the global sensor_data dictionary
            global sensor_data
            sensor_data['temperature'] = data.get('sensor1')
            sensor_data['speed'] = data.get('sensor2')
            sensor_data['level'] = data.get('sensor3')
            
            print("--- Received data from ESP8266 ---")
            print(f"Location: {data.get('location')}")
            print(f"Sensor 1 (Temperature): {sensor_data['temperature']}")
            print(f"Sensor 2 (Speed): {sensor_data['speed']}")
            print(f"Sensor 3 (Level): {sensor_data['level']}")

            return jsonify({'message': 'Data received and updated successfully'}), 200
        else:
            return jsonify({'error': 'Invalid JSON data'}), 400
    except Exception as e:
        print(f"Error receiving data: {e}")
        return jsonify({'error': 'Internal server error'}), 500

# Route to serve the sensor data to the frontend dashboard
@app.route('/api/sensors', methods=['GET'])
def get_sensor_data():
    """Provides the latest sensor data to the frontend."""
    # Return the global sensor_data as a JSON response
    return jsonify(sensor_data)

# Route to serve the main HTML dashboard page
@app.route('/', methods=['GET'])
def index():
    """Serves the main dashboard HTML page."""
    # Flask will automatically look for 'index.html' in the 'templates' folder
    return render_template('index.html')

# Start the Flask development server
if __name__ == '__main__':
    # 'host=0.0.0.0' makes the server accessible on the local network
    # so the NodeMCU can connect to it.
    app.run(host='0.0.0.0', port=4000)
