# LSB-Image-Steganography

**LSB Image Steganography** is a C-based project that implements Least Significant Bit (LSB) encoding and decoding techniques to hide secret information inside image files. The project allows you to encode a secret message or file inside an image and later decode it back without noticeable changes to the image.

## 💻 Skills gained
      • Bitwise Operations • File Handling (Binary I/O) 

## 🔧 Tools:
       GCC, VS Code, Linux Terminal

##  🧠 How It Works
1) **Encoding**

   -> Reads the source image pixel data.

   -> Replaces the **least significant bits** of each pixel with bits of the secret data.

   -> Writes the modified pixels to a new image.

3) **Decoding**

   -> Reads the LSBs from that new image.

   -> Extracts the original hidden message or file.
    
