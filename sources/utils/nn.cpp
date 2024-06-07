#include "matrix.hpp"
#include <ctime>
#include <map>
#include <string>
#include "bsl/serialization_std.hpp"
#include "random.hpp"
#include "nn.hpp"
#include "math.hpp"

namespace ActivationFunction {
	using namespace Math;

	static std::map<std::string, ActivationFunction::Ptr> s_Functions{
		{"Sigmoid", Sigmoid},
		{"Tanh", tanh},
		{"None", None}
	};

	ActivationFunction::Ptr Find(const std::string& name) {
		auto it = s_Functions.find(name);

		if(it == s_Functions.end())
			return (assert(false), None);

		return it->second;
	}
}



Layer::Layer(Matrix<float> weights, Matrix<float> biases, const std::string &function_name):
	m_Weights(std::move(weights)),
	m_Biases(std::move(biases)),
	m_FunctionName(function_name),
	m_Function(ActivationFunction::Find(function_name))
{}

Layer::Layer(int input_size, int output_size, ActivationFunction::Ptr function, const std::string &function_name) :
		m_Weights(
			Matrix<float>::Random(input_size, output_size, -20, 20)
		),
		m_Biases(
			Matrix<float>::Random(1, output_size, -20, 20)
		),
		m_Function(function),
		m_FunctionName(function_name)
	{}

Matrix<float> Layer::Do(const Matrix<float>& input)const {
	auto res = input * m_Weights - m_Biases;
	res.ForEach([=](float& e) {
		e = m_Function(e);
	});
	return res;
}

const Matrix<float>& Layer::Weights()const {
	return m_Weights;
}

const Matrix<float>& Layer::Biases()const {
	return m_Biases;
}

Matrix<float>& Layer::Weights(){
	return m_Weights;
}

Matrix<float>& Layer::Biases(){
	return m_Biases;
}


const std::string& Layer::FunctionName()const {
	return m_FunctionName;
}

ActivationFunction::Ptr Layer::Function()const {
    return m_Function;
}

bool Layer::IsComplete()const {
	return Weights().Count() && Biases().Count();
}

void Layer::Mix(Matrix<float>& output, const Matrix<float>& left, const Matrix<float>& right, float rate) {
	output.ForEachIndexed([=](float& num, size_t i, size_t j) {
		auto a = left.Get(i, j);
		auto b = right.Get(i, j);
		num = (GetRandom<float>(0, 1) < rate) ? a : b;
	});
}

Layer Layer::Crossover(const Layer& parent1, const Layer& parent2) {
	assert(parent1.IsComplete() && parent2.IsComplete());

	Layer child(parent1.m_Weights.N(), parent1.m_Weights.M(), parent1.m_Function, parent1.m_FunctionName); // Create a child layer with the same size and function

	Mix(child.m_Weights, parent1.m_Weights, parent2.m_Weights);
	Mix(child.m_Biases, parent1.m_Biases, parent2.m_Biases);

	return child;
}

Layer Layer::MutateLayer(const Layer& layer, float chance, float range) {
	assert(layer.IsComplete());

    Layer mutatedLayer(layer);

	auto mutator = [=](float &e) {
        if (GetRandom<float>(0, 1) < chance) {
            e += GetRandom<float>(-range, range); // Adjust mutation strength as needed
        }
    };
    
    mutatedLayer.m_Weights.ForEach(mutator);
    mutatedLayer.m_Biases.ForEach(mutator);

    return mutatedLayer;
}


NeuralNetwork::NeuralNetwork(std::vector<Layer> model, std::vector<int> topology, std::vector<std::string> functions):
	m_Model(std::move(model)),
	m_Topology(std::move(topology)),
	m_Functions(std::move(functions))
{}

NeuralNetwork::NeuralNetwork(std::vector<int> topology, std::vector<std::string> functions) :
	m_Topology(topology),
	m_Functions(functions)
{
	for (int i = 0; i < topology.size() - 1; i++) {
		auto function_name = functions.begin()[i];

		m_Model.emplace_back(topology.begin()[i], topology.begin()[i + 1], ActivationFunction::Find(function_name), function_name);
	}
}

Matrix<float> NeuralNetwork::Do(Matrix<float> Input)const{
	for (const auto& layer : m_Model) {
		Input = layer.Do(Input);
	}
	return Input;
}

float ComputeMeanSquaredError(const Matrix<float>& prediction, const Matrix<float>& target) {
    Matrix<float> diff = prediction - target;
    float mse = 0.0f;
    size_t count = diff.Count();
    for (size_t i = 0; i < count; ++i) {
        mse += diff.Data()[i] * diff.Data()[i];
    }
    return mse / static_cast<float>(count);
}

Matrix<float> NeuralNetwork::MeanSquaredErrorDerivative(const Matrix<float>& prediction, const Matrix<float>& target) {
    Matrix<float> result = prediction - target;
    result.ForEach([](float& val) { val *= 2.0f; });
    return result;
}

float NumericalDerivative(ActivationFunction::Ptr func, float x) {
    const float epsilon = 1e-5;
    return (func(x + epsilon) - func(x - epsilon)) / (2.0f * epsilon);
}


Matrix<float> ActivationFunctionDerivative(ActivationFunction::Ptr func, const Matrix<float>& mat) {
    Matrix<float> result(mat.N(), mat.M());
    for (size_t i = 0; i < mat.N(); ++i) {
        for (size_t j = 0; j < mat.M(); ++j) {
            result[i][j] = NumericalDerivative(func, mat[i][j]);
        }
    }
    return result;
}

float NeuralNetwork::Backpropagation(const std::vector<std::pair<Matrix<float>, Matrix<float>>>& dataset, float learning_rate) {
    float totalError = 0.0f;
    
    for (const auto& data : dataset) {
        Matrix<float> input = data.first;
        Matrix<float> target = data.second;

        // Forward pass
        std::vector<Matrix<float>> layer_inputs;
        std::vector<Matrix<float>> layer_outputs = { input };
        
        for (const auto& layer : m_Model) {
            input = layer.Do(input);
            layer_inputs.push_back(input);
            layer_outputs.push_back(input);
        }

        Matrix<float> output = layer_outputs.back();
        totalError += ComputeMeanSquaredError(output, target);

        // Backward pass
        Matrix<float> error = MeanSquaredErrorDerivative(output, target);
        
        for (int i = m_Model.size() - 1; i >= 0; --i) {
            const Matrix<float>& layer_output = layer_outputs[i + 1];
            const Matrix<float>& layer_input = layer_outputs[i];
            Layer& layer = m_Model[i];

            // Compute gradients
            Matrix<float> output_derivative = ActivationFunctionDerivative(layer.Function(), layer_output);
            Matrix<float> delta = error * output_derivative.Transpose(); // Element-wise multiplication
            Matrix<float> weight_gradient = layer_input.Transpose() * delta;
            Matrix<float> bias_gradient = delta;  // Biases are typically vectors, directly summing the delta values

            // Update weights and biases
            layer.Weights() = layer.Weights() - (weight_gradient * learning_rate);
            layer.Biases() = layer.Biases() - (bias_gradient.Transpose() * learning_rate);
            
            // Compute error for next layer if not the first layer
            if (i > 0) {
                error = delta * layer.Weights().Transpose();
            }
        }
    }
    
    return totalError / dataset.size();
}

NeuralNetwork NeuralNetwork::Crossover(const NeuralNetwork& parent1, const NeuralNetwork& parent2) {
	NeuralNetwork child(parent1.m_Topology, parent1.m_Functions); // Create a child network with the same topology and functions

	for (size_t i = 0; i < parent1.m_Model.size(); ++i) {
		child.m_Model[i] = Layer::Crossover(parent1.m_Model[i], parent2.m_Model[i]);
	}

	return child;
}

NeuralNetwork NeuralNetwork::MutateNetwork(const NeuralNetwork& network, float chance, float range) {
    NeuralNetwork mutatedNetwork(network);

    for (auto& layer : mutatedNetwork.m_Model) {
        layer = Layer::MutateLayer(layer, chance, range);
    }

    return mutatedNetwork;
}

const std::vector<Layer>& NeuralNetwork::Layers()const {
	return m_Model;
}

const std::vector<int>& NeuralNetwork::Topology()const {
	return m_Topology;
}

const std::vector<std::string>& NeuralNetwork::Functions()const {
	return m_Functions;
}

