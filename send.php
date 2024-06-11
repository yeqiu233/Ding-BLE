<?php
// 这个文件部署在服务器上
// 1.接收采集节点发送的Manufacturer Data数据并保存
// 2.返回模拟节点最新的Manufacturer Data数据

// 定义密码
$password = "your password";

// 存储文件路径
$storageFilePath = 'latest_data.txt';

// 从GET参数中获取密码并验证
$queryParams = $_GET;
if (!isset($queryParams['password']) || $queryParams['password'] !== $password) {
    http_response_code(401); // Unauthorized
    echo "Incorrect password.";
    exit;
}

// 接口1：接收一个字符串并存储到文件
if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($queryParams['data'])) {
    file_put_contents($storageFilePath, $queryParams['data']);
    http_response_code(201); // Created
    echo "Data received and stored successfully.";
    exit;
}

// 接口2：从文件返回最近一次接收到的字符串数据
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    if (file_exists($storageFilePath)) {
        $lastModifiedDate = filemtime($storageFilePath);
        $currentTimestamp = time();
        if (($currentTimestamp - $lastModifiedDate) > 120) { // 检查是否超过2分钟未更新
            http_response_code(408); // Request Timeout
            echo "No recent data available within the last 2 minutes.";
        } else {
            http_response_code(200);
            echo "data:".file_get_contents($storageFilePath);
        }
    } else {
        http_response_code(404);
        echo "No data available.";
    }
    exit;
}

http_response_code(405); // Method Not Allowed
echo "Unsupported request method.";
?>
