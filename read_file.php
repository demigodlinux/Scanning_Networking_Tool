<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

$dir = "/"; // Specify the directory you want to list

// Get the list of files in the directory
$files = scandir($dir);

// Check if the directory exists
if ($files === false) {
    http_response_code(500);
    echo json_encode(['error' => 'Failed to read directory.']);
    exit;
}

// Filter out directories and hidden files
$files = array_filter($files, function ($file) {
    return is_file('/' . $file) && !in_array($file, ['.', '..', '.gitignore']);
});

// Retrieve additional information for each file
$fileDetails = array_map(function ($file) {
    $filePath = '/' . $file; // Use the correct file path
    $fileInfo = pathinfo($filePath);
    $size = filesize($filePath);
    $createdDate = date("Y-m-d H:i:s", filemtime($filePath));

    return [
        'name' => $file,
        'size' => $size,
        'createdDate' => $createdDate,
        'type' => isset($fileInfo['extension']) ? $fileInfo['extension'] : null, // Use isset to handle cases where extension is not available
    ];
}, $files);

// Return the list as JSON
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type");
header('Content-Type: application/json');

// Check if JSON encoding is successful
$jsonData = json_encode(array_values($fileDetails), JSON_PRETTY_PRINT); // JSON_PRETTY_PRINT for a more readable JSON
if ($jsonData === false) {
    http_response_code(500);
    echo json_encode(['error' => 'Failed to encode JSON.']);
    exit;
}

echo $jsonData;
?>
