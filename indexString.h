const char* htmlString = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Directory Listing</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f8f9fa;
            text-align: center;
        }

        header {
            background-color: #343a40;
            color: #ffffff;
            padding: 1em;
        }

        h2 {
            color: #343a40;
            margin-top: 20px;
        }

        form {
            margin-bottom: 20px;
        }

        input[type="file"] {
            padding: 10px;
            border: 1px solid #ced4da;
            border-radius: 5px;
            background-color: #f1f3f5;
        }

        input[type="submit"] {
            background-color: #555;
            color: #ffffff;
            padding: 10px 15px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }

        input[type="submit"]:hover {
            background-color: #777;
        }

        h1 {
            color: #343a40;
            margin-top: 20px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }

        th, td {
            border: 1px solid #ced4da;
            padding: 12px;
            text-align: left;
        }

        th {
            background-color: #555;
            color: #ffffff;
        }

        button {
            padding: 8px 12px;
            margin-right: 5px;
            cursor: pointer;
            background-color: #28a745;
            color: #ffffff;
            border: none;
            border-radius: 4px;
        }

        button:hover {
            background-color: #218838;
        }

        footer {
            background-color: #343a40;
            color: #ffffff;
            text-align: center;
            padding: 1em;
            position: fixed;
            bottom: 0;
            width: 100%;
        }
    </style>
</head>
<body>

    <h2>Upload a File</h2>
    <form id="uploadForm" enctype="multipart/form-data">
        <input type="file" id="fileInput" name="file" required>
        <input type="submit" value="Upload">
    </form>

    <h1>File List</h1>
    <table>
        <thead>
            <tr>
                <th>File Name</th>
                <th>File Type</th>
                <th>File Size</th>
                <th>Actions</th>
            </tr>
        </thead>
        <tbody id="fileList"></tbody>
    </table>

    <script>
      let customHeaders = new Headers();
      customHeaders.append('Content-Type', 'multipart/form-data');
      document.getElementById('uploadForm').addEventListener('submit', function (event) {
            event.preventDefault();

            // Get the file input element
            const fileInput = document.getElementById('fileInput');

            const fileExtension = fileInput.value.split('.').pop().toLowerCase();

            // Array of allowed extensions
            const allowedExtensions = ['txt', 'html', 'json', 'js', 'css', 'xml'];

            // Check if file has an allowed extension
            if (!allowedExtensions.includes(fileExtension)) {
                // If not, display an error message and stop the function
                alert('Invalid file type. Please upload a .txt, .html, .json, .js, .css, or .xml file.');
                return;
            }

            const file = fileInput.files[0];
            const fileType = file.name.split('.')[1];
            const fileData = {};
            if (file) {
              const reader = new FileReader();
      
              reader.onload = function (e) {
                // The file content is available as a buffer in e.target.result
                const buffer = e.target.result;
                const buffArray = [];

                const uint8Array = new Uint8Array(buffer);
                for (let key in uint8Array){
                  buffArray.push(uint8Array[key]);
                }
                // For example, convert the buffer to a string
                const text = new TextDecoder().decode(buffer);
                console.log('File content as text:', text);

                // Create a FormData object
                const formData = new FormData();
                formData.append('file', file);
                fileData['name'] = file.name;
                fileData['size'] = file.size;
                fileData['type'] = fileType;
                // Make a fetch request to upload the file
                fetch('/Directory', {
                    method: 'POST',
                    body: formData,
                    headers: {
                        'Content-Type': 'application/json',
                        'File': JSON.stringify(fileData),
                        'Buffer': JSON.stringify({buffer: buffArray}),
                    },
                })
                .then(response => response.json())
                .then(data => {
                    console.log('Server response:', data);
                })
                .catch(error => console.error('Error:', error));
              };
              // Read the file as an ArrayBuffer
              reader.readAsArrayBuffer(file);
            }
        });
        // Fetch the JSON data from the ESP32 server
        fetch('/data.json')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`Network response was not ok; status: ${response.status}`);
                }
                return response.json();
            })
            .then(data => {
                // Process the JSON data and update the HTML
                const fileList = document.getElementById('fileList');

                data.forEach(file => {
                    const row = fileList.insertRow();
                    const cellName = row.insertCell(0);
                    const cellType = row.insertCell(1);
                    const cellSize = row.insertCell(2);
                    const cellActions = row.insertCell(3);
                    cellName.textContent = file.name;
                    cellType.textContent = file.type;
                    cellSize.textContent = file.size;

                    // Create download, update, and delete buttons
                    if(('.html .json .txt .xml .css .js').includes(file.type)){
                      const downloadBtn = document.createElement('button');
                      downloadBtn.textContent = 'Download';
                      downloadBtn.addEventListener('click', () => downloadFile(file.name));

                      const updateBtn = document.createElement('button');
                      updateBtn.textContent = 'Update';
                      updateBtn.addEventListener('click', () => updateFile(file.name));

                      const deleteBtn = document.createElement('button');
                      deleteBtn.textContent = 'Delete';
                      deleteBtn.addEventListener('click', () => deleteFile(file.name));

                      cellActions.appendChild(downloadBtn);
                      cellActions.appendChild(updateBtn);
                      cellActions.appendChild(deleteBtn);
                    }
                    else{
                      cellActions.textContent = 'Not to be Modified';
                    }
                });
            })
            .catch(error => console.error('Error fetching data:', error));

        function downloadFile(fileName) {
            // Implement download functionality
            console.log(`Downloading file: ${fileName}`);
        }

        function updateFile(fileName) {
            // Implement update functionality
            console.log(`Updating file: ${fileName}`);
        }

        function deleteFile(fileName) {
            // Implement delete functionality
            console.log(`Deleting file: ${fileName}`);
        }
    </script>
</body>
</html>

)";