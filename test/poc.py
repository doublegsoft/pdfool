from google import genai
from google.genai import types

client = genai.Client(api_key="AIzaSyCOnNGw2qvxXu-qC3-w9yEujumePSY2v-k")

myfile1 = client.files.upload(file="/Users/christian/Downloads/bitems1.png")
myfile2 = client.files.upload(file="/Users/christian/Downloads/bitems2.png")

response = client.models.generate_content(
    model="gemini-2.5-flash",
    contents=[
        myfile1, myfile2,
        "\n\n",
        "比较两张图片内容的相似度",
    ],
)
print(response.text)
