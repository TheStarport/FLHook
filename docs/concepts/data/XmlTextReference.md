---
author: "Laz, FriendlyFire"
date: "2024-05-16"
title: XML Text Reference
---

# XML Text Reference

### Context

Freelancer uses XML nodes in order to structure text within the game, and this extends to chat messages sent to/fro the server to clients.
Functions like `ClientId::Message()` automatically handle the XML wrapping for you and offer a number of different format options for customisation.

If the options provided are not enough for your use case, there is an additional function on the ClientId that allows you to send manually created XML.
`ClientId::MessageCustomXml()` allows you to send in a custom buffer of XML to send to the client, but it must conform to the FL's XML standard, or it will not be parsed and sent.

### Valid Nodes

Text is enclosed in `<TEXT></TEXT>` tags while the format can be changed with `<TRA .../>`. Nodes names must be written in all caps! Be sure to replace the following characters within a text node:
```
    < → &#60;
    > → &#62;
    & → &#38;
```

Valid XML Nodes:
```xml
    <TEXT></TEXT>
    <JUST loc=left|center|right/>
    <PARA/>
    <TRA data="*" mask="-1"/>
```

JUST and TRA will apply their effect on all TEXT blocks until a new effect is defined.

### TRA node syntax

The data field of a `TRA` node consists of an RGB value along with format specifications:
```xml
    <TRA data="0xBBGGRRFF" mask="-1"/>
```
* `BB` is the blue value
* `GG` is the green value
* `RR` is the red value
* `FF` is the format value

(All in hexadecimal representation)

Format flags are:
| bin        | hex  | dec   | Formatting |
| ---------- | ---- | ----- | ---------- |
| `00000001` | `1 ` | `1  ` | Bold       |
| `00000010` | `2 ` | `2  ` | Italic     |
| `00000100` | `4 ` | `4  ` | Underline  |
| `00001000` | `8 ` | `8  ` | Big        |
| `00010000` | `10` | `16 ` | Big & wide |
| `00100000` | `20` | `32 ` | Very big   |
| `01000000` | `40` | `64 ` | Smoothest? |
| `10000000` | `80` | `128` | Smoother?  |
| `10010000` | `90` | `144` | Small      |

Simply add the flags to combine them (e.g. 7 = bold/italic/underline), and remember to represent them as a hexadecimal value.

Examples:
* `client.MessageCustomXml(LR"(<TRA data="0x1919BD01" mask="-1"/><TEXT>You are a very nice person.</TEXT>)"` This is similar to the standard death message (which is shown in bold red).
* `client.MessageCustomXml(LR"(<TRA data="0xFF000003" mask="-1"/><TEXT>Hello</TEXT><TRA data="0x00FF0008" mask="-1"/> <TEXT>World</TEXT>)"` This will show "Hello World" ("Hello" will be blue/bold/italic and "World" green/big).