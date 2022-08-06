from concurrent import futures
import time
import grpc
import src.rpc.python.c2nl_pb2 as c2nl_pb2, src.rpc.python.c2nl_pb2_grpc as c2nl_pb2_grpc

import argparse, os, json
import torch
import torch.nn as nn
from src.utils import logger, convert_examples_to_features, Example
from src.model import Seq2Seq
from torch.utils.data import TensorDataset, DataLoader, SequentialSampler
from transformers import RobertaConfig, RobertaModel, RobertaTokenizer



class Code2NL(c2nl_pb2_grpc.Code2NLServicer):
    def summarize(self, request, context):
        source = request.code
        example = [
                Example(
                    source=source,
                    target=None,
                )
            ]
        t0 = time.time()
        message, length = inference(data=get_features(example))
        t1 = time.time()
        result = {
                'message' : message,
                'time' : (t1 - t0),
                'device' : args.device_name,
                'length' : length
            }

        logger.info(json.dumps(result, indent=4))

        return c2nl_pb2.Summary(summary = message[0])


def serve():
    # 启动 rpc 服务
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    c2nl_pb2_grpc.add_Code2NLServicer_to_server(Code2NL(), server)
    server.add_insecure_port('[::]:50051')
    server.start()

    logger.info("GRPC Started!")

    try:
        while True:
            time.sleep(60*60*24) # one day in seconds
    except KeyboardInterrupt:
        server.stop(0)


############################################################################
############################################################################
############################################################################


def inference(data):
    # Calculate bleu
    eval_sampler = SequentialSampler(data)
    eval_dataloader = DataLoader(data, sampler=eval_sampler, batch_size=len(data))

    args.model.eval()
    p = []
    for batch in eval_dataloader:
        batch = tuple(t.to(args.device) for t in batch)
        source_ids, source_mask = batch
        with torch.no_grad():
            preds = args.model(source_ids=source_ids, source_mask=source_mask)
            for pred in preds:
                t = pred[0].cpu().numpy()
                t = list(t)
                if 0 in t:
                    t = t[: t.index(0)]
                text = args.tokenizer.decode(t, clean_up_tokenization_spaces=False)
                p.append(text)
    return (p, source_ids.shape[-1])


def build_model(model_class, config, tokenizer):
    encoder = model_class(config=config)
    decoder_layer = nn.TransformerDecoderLayer(
        d_model=config.hidden_size, nhead=config.num_attention_heads
    )
    decoder = nn.TransformerDecoder(decoder_layer, num_layers=6)
    model = Seq2Seq(
        encoder=encoder,
        decoder=decoder,
        config=config,
        beam_size=args.beam_size,
        max_length=args.max_target_length,
        sos_id=tokenizer.cls_token_id,
        eos_id=tokenizer.sep_token_id,
    )

    assert os.path.exists("./src/models/pytorch_model.bin"), "Weight is not downloaded."

    model.load_state_dict(
        torch.load(
            "./src/models/pytorch_model.bin",
            map_location=torch.device("cpu"),
        ),
        strict=False,
    )
    return model


def get_features(examples):
    features = convert_examples_to_features(
        examples, args.tokenizer, args, stage="test"
    )
    all_source_ids = torch.tensor(
        [f.source_ids[: args.max_source_length] for f in features], dtype=torch.long
    )
    all_source_mask = torch.tensor(
        [f.source_mask[: args.max_source_length] for f in features], dtype=torch.long
    )
    return TensorDataset(all_source_ids, all_source_mask)


def main(args):
    config = RobertaConfig.from_pretrained("microsoft/codebert-base")
    args.tokenizer = RobertaTokenizer.from_pretrained(
        "microsoft/codebert-base", do_lower_case=False
    )
    args.model = build_model(
        model_class=RobertaModel, config=config, tokenizer=args.tokenizer
    ).to(args.device)

    logger.info("Model Loaded!")
    serve()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="")
    parser.add_argument(
        "--max_source_length",
        default=512,
        type=int,
        help="The maximum total source sequence length after tokenization. Sequences longer "
        "than this will be truncated, sequences shorter will be padded.",
    )
    parser.add_argument(
        "--max_target_length",
        default=128,
        type=int,
        help="The maximum total target sequence length after tokenization. Sequences longer "
        "than this will be truncated, sequences shorter will be padded.",
    )
    parser.add_argument(
        "--beam_size", default=10, type=int, help="beam size for beam search"
    )
    parser.add_argument("--no_cuda", action='store_true', help="Avoid using CUDA when available")

    parser.add_argument("--host", type=str, default="0.0.0.0")
    parser.add_argument("--port", type=int, default=20000)

    args = parser.parse_args()

    args.device_name = "cuda" if torch.cuda.is_available() and not args.no_cuda else "cpu"
    args.device = torch.device(args.device_name)
    args.beam_size = (args.beam_size if torch.cuda.is_available() and not args.no_cuda else 1)

    logger.info(args)

    main(args)