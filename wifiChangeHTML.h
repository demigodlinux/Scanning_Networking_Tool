const char* wifiHtmlString = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Change connection string</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            margin: 20px;
            text-align: center;
        }

        h1 {
            color: #333;
        }

        form {
            max-width: 400px;
            margin: 20px auto;
            background-color: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }

        label {
            display: block;
            margin-bottom: 8px;
            color: #333;
        }

        input {
            width: 100%;
            padding: 10px;
            margin-bottom: 15px;
            box-sizing: border-box;
            border: 1px solid #ccc;
            border-radius: 4px;
        }

        input[type="button"] {
            background-color: #555;
            color: #fff;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        input[type="button"]:hover {
            background-color: #777;
        }
    </style>
</head>

<body>
    <h1>Change connection string</h1>

    <form id="connectionForm">
        <label for="wifiSSID">WI-FI SSID:</label>
        <input id="wifiSSID" type="text" name="wifiSSID" placeholder="Enter Wi-Fi SSID" required>

        <label for="wifiPass">WI-FI Password:</label>
        <input id="wifiPass" type="text" name="wifiPass" placeholder="Enter Wi-Fi Password" required>

        <input type="button" value="Submit" onclick='updateConnectionString()'>
    </form>

    <footer>
        <p>&copy; 2023 ESP32 Server. All rights reserved.</p>
    </footer>

    <script>
        fetch('/ConnectionString.json')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`Network response was not ok; status: ${response.status}`);
                }
                return response.json();
            })
            .then(data => {
              document.getElementById('wifiSSID').value = data.Wifi_SSID;
              document.getElementById('wifiPass').value = data.Wifi_Pass;
            });

        function updateConnectionString() {
            // Get the value from the text area
            var ssid = document.getElementById('wifiSSID').value;
            var pass = document.getElementById('wifiPass').value;
            var newConnString = JSON.stringify({Wifi_SSID: ssid, Wifi_Pass: pass})
            // You can perform further actions with the newConnectionString here
            // You may also want to send the newConnectionString to a server using AJAX or perform other actions
            fetch('/SaveWifi', {
                method: 'POST',
                body: newConnString, // Send data as JSON
                headers: {
                    'Content-Type': 'application/json',
                    'Content-Body': newConnString
                },
            })
            .then(response => response.json())
            .then(data => {
                console.log('Server response:', data);
            })
            .catch(error => console.error('Error:', error));
        }
    </script>
</body>

</html>
)";