# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: c2nl.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='c2nl.proto',
  package='',
  syntax='proto3',
  serialized_options=None,
  serialized_pb=_b('\n\nc2nl.proto\"\x14\n\x04\x43ode\x12\x0c\n\x04\x63ode\x18\x01 \x01(\t\"\x1a\n\x07Summary\x12\x0f\n\x07summary\x18\x01 \x01(\t2)\n\x07\x43ode2NL\x12\x1e\n\tsummarize\x12\x05.Code\x1a\x08.Summary\"\x00\x62\x06proto3')
)




_CODE = _descriptor.Descriptor(
  name='Code',
  full_name='Code',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='code', full_name='Code.code', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=14,
  serialized_end=34,
)


_SUMMARY = _descriptor.Descriptor(
  name='Summary',
  full_name='Summary',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='summary', full_name='Summary.summary', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=36,
  serialized_end=62,
)

DESCRIPTOR.message_types_by_name['Code'] = _CODE
DESCRIPTOR.message_types_by_name['Summary'] = _SUMMARY
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

Code = _reflection.GeneratedProtocolMessageType('Code', (_message.Message,), dict(
  DESCRIPTOR = _CODE,
  __module__ = 'c2nl_pb2'
  # @@protoc_insertion_point(class_scope:Code)
  ))
_sym_db.RegisterMessage(Code)

Summary = _reflection.GeneratedProtocolMessageType('Summary', (_message.Message,), dict(
  DESCRIPTOR = _SUMMARY,
  __module__ = 'c2nl_pb2'
  # @@protoc_insertion_point(class_scope:Summary)
  ))
_sym_db.RegisterMessage(Summary)



_CODE2NL = _descriptor.ServiceDescriptor(
  name='Code2NL',
  full_name='Code2NL',
  file=DESCRIPTOR,
  index=0,
  serialized_options=None,
  serialized_start=64,
  serialized_end=105,
  methods=[
  _descriptor.MethodDescriptor(
    name='summarize',
    full_name='Code2NL.summarize',
    index=0,
    containing_service=None,
    input_type=_CODE,
    output_type=_SUMMARY,
    serialized_options=None,
  ),
])
_sym_db.RegisterServiceDescriptor(_CODE2NL)

DESCRIPTOR.services_by_name['Code2NL'] = _CODE2NL

# @@protoc_insertion_point(module_scope)
