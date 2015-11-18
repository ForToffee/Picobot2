# TweetBot

Connects to channel https://thingspeak.com/channels/65715 and gets last tweet (http://api.thingspeak.com/channels/65715/feed.json to see all tweets)

Command set is currently

- Fx = forward x seconds
- Bx = backward x seconds
- Lx = turn left x 10th seconds
- Rx = turn right x 10th seconds
- [rrggbb] = light both pixels hex RGB value


tweets must contain #tweetcontrol #bot
so

 #tweetcontrol #bot F1L2B2R1[FFFFFF]
 
would result in 

- Forward 1 sec, 
- turn left for 0.2 sec, 
- backwards 2 secs, 
- turn right 0.1 sec, 
- set pixels white

WARNING! There's no sanity checking on the input data so if you get a bum response then the bot will attempt to process it!
