# whisper
Whisper: a steganographic tool for hiding data inside audio

What is steganography?

Simply put, steganography is concealing data within other data. Contrary to the case of cryptography, data concealed by steganography is completely vulnerable if detected. Nevertheless, the power of steganography is not so much its security but rather its obscurity, that is, the fact that no one may be looking for data concealed in this way. Arguably, if ever quantum computing renders useless the security of current cryptography, there may be some time before the advent of quantum cryptography that obscurity may be our only remaining choice.    

What is it that Whisper does?

In its current version, Whisper packs into a WAV audio file any arbitrary file for which the host WAV file has sufficient space. Determining the required space is a complicated matter since currently Whisper embeds the data at one bit per sample, but only in the case of sample values within a specific range. This means that either a WAV file has to be analyzed ahead of Whisper encoding, or instead proceeding with the encoding has to be abandoned on discovery of insufficient space in the destination WAV file. Note that in a 16-bit WAV file, no fewer than 8 samples are required to store a byte of hidden data, meaning a "best-case" scenario would be a ratio in bytes of 1:16. But only in the most contrived scenarios could such a "best-case" be even close. 

Can I convert my enocded WAV files to MP3 or some other lossy compression format?

Of course you could, but the encoded data would be unrecoverably destroyed in the process. 

When will Whisper support compressed audio? 

Probably not anytime soon. Maybe never. Compressed audio, lossy or not, complicates determining whether the encoded data might be noticed. It might be possible to convert your encoded WAV file to one with lossless compression, but your mileage may vary. The introduction, omission, or alteration of even one sample may be enough to completely corrupt your encoded data.      

What kind of encryption does Whisper employ? 

None. Whisper is not intended for encryption. For it to be would be a presumptuous error, especially when considering that there is nothing preventing one from "whispering" an encrypted file into a WAV and then even encrypting the result!

So, isn't it a huge vulnerability that anyone having Whisper can easily decode any "whispered" data? 

Yes, if they know/bother to look, of course. If this is a concern, encrypt your files before encoding them. 