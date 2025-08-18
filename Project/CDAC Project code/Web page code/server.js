const express = require('express');
const path = require('path');
const app = express();
const port = 4000;

// Middleware to parse JSON bodies from POST requests
app.use(express.json());

// A global variable to store the latest sensor data
let sensorData = {
    temperature: 0.0,
    speed: 0.0,
    level: 0.0
};

// API Endpoint for the ESP8266 to POST data
app.post('/sensors', (req, res) => {
    const { location, sensor1, sensor2, sensor3 } = req.body;
    console.log(`\n--- Received data from ESP8266 ---`);
    console.log(`Location: ${location}`);
    console.log(`Sensor 1 (Temperature): ${sensor1}`);
    console.log(`Sensor 2 (Speed): ${sensor2}`);
    console.log(`Sensor 3 (Level): ${sensor3}`);
    sensorData.temperature = sensor1;
    sensorData.speed = sensor2;
    sensorData.level = sensor3;
    res.status(200).send('Data received and updated successfully');
});

// NEW ENDPOINT: This will return the server's IP address.
// This is not used by your current NodeMCU code but is a useful addition
// for debugging and future-proofing.
app.get('/ip', (req, res) => {
    // req.ip might return '::1' for localhost, so it's not ideal for network discovery.
    // For local network, the hardcoded IP is often necessary.
    // However, for more complex setups, this endpoint could be modified.
    res.send(req.ip);
});

// API Endpoint for the Frontend to GET data
app.get('/api/sensors', (req, res) => {
    res.json(sensorData);
});

// Serve the HTML Page to the browser
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

app.listen(port, () => {
    console.log(`Server listening at http://10.91.73.251${port}`);
    console.log(`Dashboard available at http://10.91.73.251${port}`);
});
