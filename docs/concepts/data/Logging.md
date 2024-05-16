---
author: "Laz"
date: "2024-05-16"
title: "Logging"
---

All logging is performed through the use of the `Logger` class. Use of this class is thread-safe as it utilities a concurrent queue. This does mean there is a very low chance of logs appearing in the wrong order when being done from numerous different threads, but it is very unlikely.

FLHook 5.0 onwards has been designed to use a sidecar application, called FLAdmin, that off-loads a lot of functionality including logging.
However, if FLAdmin is not being used the game is still capable of logging to a file.

TODO: Finish