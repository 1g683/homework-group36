import requests

url = 'https://api.blockcypher.com/v1/btc/test3/addrs/mybeGUL4pq6crixQHWEuDNquGUxDgWtpKD/full?limit=50'
response = requests.get(url)

# 将响应内容保存到一个 .md 文件中
with open("result.md", "w", encoding="utf-8") as f:
        f.write(response.text)
print("Data saved to result.md successfully.")
