const char* mainHtml = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Server</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f4;
            text-align: center;
        }

        header {
            background-color: #333;
            color: white;
            padding: 1em;
        }

        .page {
            display: none;
        }

        .page.active {
            display: block;
        }

        section {
          padding: 10px; /* Reduced padding */
          margin: 10px; /* Reduced margin */
          background-color: white;
          border-radius: 4px; /* Reduced border-radius */
          box-shadow: 0 0 5px rgba(0, 0, 0, 0.1); /* Reduced box-shadow */
          display: inline-block;
          width: 320px; /* Increased width by 50 pixels */
          text-align: left;
          font-size: 14px; /* Reduced font size */
        }

        .pagination {
            margin-top: 10px; /* Reduced margin-top */
        }

        .pagination button {
            background-color: #555;
            color: white;
            padding: 6px 12px; /* Reduced padding */
            border: none;
            border-radius: 3px; /* Reduced border-radius */
            cursor: pointer;
            font-size: 12px; /* Reduced font size */
        }

        .pagination button:hover {
            background-color: #777;
        }

        footer {
            background-color: #333;
            color: white;
            text-align: center;
            padding: 1em;
            position: fixed;
            bottom: 0;
            width: 100%;
            font-size: 12px; /* Reduced font size */
        }
    </style>
</head>
<body>

    <header>
        <h1>ESP32 Server</h1>
    </header>

    <div class="page active">
        <section onclick="redirectToPage('/Directory')">
            <h2>Directory</h2>
            <p>Lists all the files in the memory card.</p>
        </section>

        <section onclick="redirectToPage('/SetWifi')">
            <h2>Change WI-FI Conn Strings</h2>
            <p>To change wifi credentials and connect to your local network, change the strings only when required!!</p>
        </section>

        <section>
            <h2>Option 3</h2>
            <p>This is the content for Option 3. Add a brief description here.</p>
        </section>
    </div>

    <div class="page">
        <section>
            <h2>Option 4</h2>
            <p>This is the content for Option 4. Add a brief description here.</p>
        </section>

        <section>
            <h2>Option 5</h2>
            <p>This is the content for Option 5. Add a brief description here.</p>
        </section>

        <section>
            <h2>Option 6</h2>
            <p>This is the content for Option 6. Add a brief description here.</p>
        </section>
    </div>

    <div class="pagination">
        <button onclick="showPage(1)">1</button>
        <button onclick="showPage(2)">2</button>
    </div>

    <footer>
        <p>&copy; 2023 ESP32 Server. All rights reserved.</p>
    </footer>

    <script>
        function showPage(pageNumber) {
            const pages = document.querySelectorAll('.page');
            pages.forEach(page => page.classList.remove('active'));
            pages[pageNumber - 1].classList.add('active');
        }

        function redirectToPage(pageURL) {
            window.location.href = pageURL;
        }
    </script>

</body>
</html>
)=====";