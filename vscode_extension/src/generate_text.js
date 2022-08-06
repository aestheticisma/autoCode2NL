"use strict";
const axios = require('axios');
const vscode = require('vscode');
const parse = require("./parse/parse").parse;
// const http = require('http');
class CodeSum {
    constructor(editor) {
        this.editor = editor;
    }
    generateCodeSum() {
        if (this.editor == undefined) {
            throw new Error("Cannot process this document. It is either too large or is not yet supported.");
        }
        const position = this.editor.selection.active;
        const document = this.editor.document.getText();
        // console.log(position);
        // console.log(document);
        const code = parse(document, position.line)
        console.log(code);
        const originText = "# The AI is generating code summaries...Please wait seconds..."
        const snippet = new vscode.SnippetString(originText);
        // const insertPosition = position.with(undefined, 0);
        const success = this.editor.insertSnippet(snippet, position);
        success.then(
            () => {
                console.log("Successfully inserted docstring");
                const docstringSnippetLength = snippet.value.split('\n').length;
                const docstringSnippetRange = new vscode.Range(
                    position.line, 0,
                    position.line + docstringSnippetLength, 0
                );
                axios.post(`http://10.8.129.27:8888/summary`, {
                    code: code.join(' ')
                }).then((response) => {
                    // console.log(response);
                    const summary = response.data.message[0];
                    console.log(summary);
                    this.editor.edit(editBuilder => {
                        editBuilder.replace(docstringSnippetRange,
                            this.editor.document.getText(docstringSnippetRange).replace(
                                `# The AI is generating code summaries...Please wait seconds...`,
                                '# ' + summary
                            )
                        );
                    });
                }

                )
                // axios.
            },

            (reason) => {
                throw new Error("Could not insert docstring: " + reason.toString());
            },
        );

        return success;
    }
    getCodeText(document, position) {

    }
}

exports.CodeSum = CodeSum;