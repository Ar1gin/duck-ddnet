[![DDraceNetwork](https://ddnet.org/ddnet-small.png)](https://ddnet.org)

# Ducks custom DDNet Client
Nothing special, just some additional features I thought would be useful to have

Feel free to copy these features in your own client

# Showcase
![Screenshot_20231104_182112](https://github.com/Ar1gin/duck-ddnet/assets/79476345/eb4d5f79-50cf-48c9-a149-18fec4d9a1f1)
![Screenshot_20231104_182230](https://github.com/Ar1gin/duck-ddnet/assets/79476345/12618c7e-cf26-4e0a-a41c-08820efb596e)

# Features
```
dc_drawstats
dc_drawdj
dc_freemouse
dc_unlockzoom
```

## Precompiled Headers

By default precompiled headers are used for common system includes, see `src/pch.h`. In CI the Ubuntu 22.04 build disables precompiled headers to ensure we don't break compilation by forgetting includes. You might want to disable precompiled headers locally instead, at the cost of slower compile times, by setting `-DPRECOMPILE_HEADERS=OFF` in `cmake`.
