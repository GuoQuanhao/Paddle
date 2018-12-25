/*Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/fluid/operators/shuffle_channel_op.h"

namespace paddle {
namespace operators {

class ShuffleChannelOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

  void InferShape(framework::InferShapeContext* ctx) const override {
    PADDLE_ENFORCE(ctx->HasInput("X"),
                   "Input(X) of ShuffleChannelOp should not be null.");
    PADDLE_ENFORCE(ctx->HasOutput("Out"),
                   "Output(Out) of ShuffleChannelOp should not be null.");

    auto input_dims = ctx->GetInputDim("X");
    PADDLE_ENFORCE(input_dims.size() == 4, "The layout of input is NCHW.");

    ctx->SetOutputDim("Out", input_dims);
  }
  /*
   protected:
    framework::OpKernelType GetExpectedKernelType(
        const framework::ExecutionContext& ctx) const override {
      return framework::OpKernelType(
          framework::ToDataType(ctx.Input<framework::Tensor>("X")->type()),
          ctx.device_context());
    }
  */
};

class ShuffleChannelOpMaker : public framework::OpProtoAndCheckerMaker {
 public:
  void Make() override {
    AddInput("X",
             "(Tensor, default Tensor<float>), "
             "the input feature data of ShuffleChannelOp, the layout is NCHW.");
    AddOutput("Out",
              "(Tensor, default Tensor<float>), the output of "
              "ShuffleChannelOp. The layout is NCHW.");
    AddAttr<int>("group", "the number of groups.")
        .SetDefault(1)
        .AddCustomChecker([](const int& group) {
          PADDLE_ENFORCE_GE(group, 1, "group should be larger than 0.");
        });

    AddComment(R"DOC(
		Shuffle Channel operator
		This operator obtains the group convolutional layer with channels shuffled.
		Firstly, divide the input channels in each group into several subgroups,
		then, feed each group in the next layer with different subgroups.

		According to the paper, "Suppose a convolution layer with G groups
		whose output has (G * N) channels, first reshape the output channel dimension into(G,N),
		transposing and then flattening it back as the input of next layer. "

		Shuffle channel operation makes it possible to build more powerful structures
		with multiple group convolutional layers.

		please get more information from the following paper:
		https://arxiv.org/pdf/1707.01083.pdf
        )DOC");
  }
};

class ShuffleChannelGradOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

  void InferShape(framework::InferShapeContext* ctx) const override {
    PADDLE_ENFORCE(ctx->HasInput(framework::GradVarName("Out")),
                   "Input(Out@Grad) should not be null");
    PADDLE_ENFORCE(ctx->HasOutput(framework::GradVarName("X")),
                   "Output(X@Grad) should not be null");

    auto input_dims = ctx->GetInputDim("X");
    PADDLE_ENFORCE(input_dims.size() == 4, "The layout of input is NCHW.");

    ctx->SetOutputDim(framework::GradVarName("X"), input_dims);
  }
  /*
   protected:
    framework::OpKernelType GetExpectedKernelType(
        const framework::ExecutionContext& ctx) const override {
      return framework::OpKernelType(
          framework::ToDataType(
                  framework::ToDataType(ctx.Input<framework::Tensor>("X")->type()),
          ctx.device_context());
    }
  */
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OPERATOR(shuffle_channel, ops::ShuffleChannelOp,
                  ops::ShuffleChannelOpMaker,
                  paddle::framework::DefaultGradOpDescMaker<true>);

REGISTER_OPERATOR(shuffle_channel_grad, ops::ShuffleChannelGradOp);

REGISTER_OP_CPU_KERNEL(
    shuffle_channel,
    ops::ShuffleChannelOpKernel<paddle::platform::CPUDeviceContext, float>,
    ops::ShuffleChannelOpKernel<paddle::platform::CPUDeviceContext, double>);

REGISTER_OP_CPU_KERNEL(
    shuffle_channel_grad,
    ops::ShuffleChannelGradOpKernel<paddle::platform::CPUDeviceContext, float>,
    ops::ShuffleChannelGradOpKernel<paddle::platform::CPUDeviceContext,
                                    double>);
