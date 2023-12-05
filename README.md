# 4061-PA4

## PA Group 98 members

Ryan Bergstein - bergs643

Marwa Osman - osman320

Basma Elaraby - elara006

## CSE computer used

csel-kh1250-09.cselabs.umn.edu

## Changes to Makefile

None for inter submission.

## Individual Contributions: 

Ryan: Created a group github repository to collectively work out of and set up the project files and README document. Wrote initial code to set up the client side as well as created the server side.

Marwa: Wrote the code to set up and connect socket in client.c as well as directory traversal/adding content to request queue. Also wrote code for sending image data to server. 

Basma: Wrote down the plan on constructing the client handling thread and sending the package according to the protocol.

##  Plan on how you are going to construct the client handling thread and how you plan to send the package according to the protocol.

Constructing the Client Handling Thread:
1. Listening for Connections: The server listens for incoming client connections.
2. Starting a Thread per Client: For each connected client, a dedicated thread (client handling thread) is initiated.
3. Handling Client Requests: This thread listens for specific operation messages from the client. When it receives an IMG_OP_ROTATE message, it indicates the client is requesting an image rotation.
4. Thread Lifecycle Management: The thread remains active, processing requests (like IMG_OP_ROTATE) until it receives an IMG_OP_EXIT message, signaling the client wishes to end the session. At this point, the thread closes the client connection and terminates.

Sending the Package According to the Protocol:
1. Packet Structure and Protocol: Communication uses a packet structure including Operation (IMG_OP_ROTATE, IMG_OP_ACK, IMG_OP_NAK, IMG_OP_EXIT), Flags (e.g., IMG_FLAG_ROTATE_180, IMG_FLAG_ROTATE_270 for rotation angles), and Image Data.
2. Receiving Image Rotation Request: Upon receiving an IMG_OP_ROTATE message with the corresponding flags for the rotation angle, the server processes the image accordingly.
3. Responding to the Client: After processing, the server sends back a packet. If successful, it includes IMG_OP_ACK and the rotated image. In case of an error, it sends IMG_OP_NAK.
4. Error Handling and Confirmation: The server handles errors during image processing by sending IMG_OP_NAK. Successful operations are confirmed with IMG_OP_ACK along with the processed image.


