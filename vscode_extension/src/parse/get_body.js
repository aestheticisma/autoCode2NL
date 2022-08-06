"use strict";
const blankLine = require("./utils").blankLine;
const indentationOf = require("./utils").indentationOf;
const preprocessLines = require("./utils").preprocessLines;

function getBody(document, linePosition) {
    const lines = document.split("\n");
    // console.log(lines);
    const body = [];
    let currentLineNum = linePosition;
    // console.log("test1:" + currentLineNum);
    const originalIndentation = getBodyBaseIndentation(lines, linePosition);
    // console.log("test2:" + originalIndentation);
    while (currentLineNum < lines.length) {
        const line = lines[currentLineNum];
        if (blankLine(line)) {
            currentLineNum++;
            continue;
        }
        if (indentationOf(line) < originalIndentation) {
            break;
        }
        body.push(line);
        currentLineNum++;
    }
    return preprocessLines(body);
}

function getBodyBaseIndentation(lines, linePosition) {
    let currentLineNum = linePosition;
    const functionDefRegex = /\s*def \w+/;
    while (currentLineNum < lines.length) {
        const line = lines[currentLineNum];
        if (blankLine(line)) {
            currentLineNum++;
            continue;
        }
        if (functionDefRegex.test(line)) {
            break;
        }
        return indentationOf(line);
    }
    return 10000;
}

exports.getBody = getBody;