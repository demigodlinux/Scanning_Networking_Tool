<?php
$dir = "./Uploads"; // Specify the directory you want to list

// Get the list of files in the directory
$files = scandir($dir);

// Filter out directories and hidden files
$files = array_filter($files, function ($file) {
    return is_file($file) && !in_array($file, ['.', '..', '.gitignore']);
});

// Retrieve additional information for each file
$fileDetails = array_map(function ($file) use ($dir) {
    $filePath = $dir . $file;
    $fileInfo = pathinfo($filePath);
    $size = filesize($filePath);
    $createdDate = date("Y-m-d H:i:s", filemtime($filePath));

    return [
        'name' => $file,
        'size' => $size,
        'createdDate' => $createdDate,
        'type' => $fileInfo['extension'],
    ];
}, $files);

// Return the list as JSON
header('Content-Type: application/json');
echo json_encode(array_values($fileDetails));
?>
