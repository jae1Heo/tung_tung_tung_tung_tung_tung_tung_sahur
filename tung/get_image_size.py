from PIL import Image

image = Image.open('images/sahur.png') # variable image stores image data

width, height = image.size # variable width, height stores read image's width and height

print(f"{width}") # prints image's width
print(f"{height}") # prints image's height
