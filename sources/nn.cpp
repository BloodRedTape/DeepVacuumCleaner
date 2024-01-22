#include "matrix.hpp"
#include <ctime>
#include <map>
#include <string>
#include "serialization.hpp"
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

const std::string& Layer::FunctionName()const {
	return m_FunctionName;
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

Layer Layer::MutateLayer(const Layer& layer, float mutationRate) {
	assert(layer.IsComplete());

    Layer mutatedLayer(layer);
    
    mutatedLayer.m_Weights.ForEach([=](float &e) {
        if (GetRandom<float>(0, 1) < mutationRate) {
            e += GetRandom<float>(-100, 100); // Adjust mutation strength as needed
        }
    });

    mutatedLayer.m_Biases.ForEach([=](float &e) {
        if (GetRandom<float>(0, 1) < mutationRate) {
            e += GetRandom<float>(-100, 100); // Adjust mutation strength as needed
        }
    });

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

Matrix<float> NeuralNetwork::Do(Matrix<float> Input) {
	for (const auto& layer : m_Model) {
		Input = layer.Do(Input);
	}
	return Input;
}

NeuralNetwork NeuralNetwork::Crossover(const NeuralNetwork& parent1, const NeuralNetwork& parent2) {
	NeuralNetwork child(parent1.m_Topology, parent1.m_Functions); // Create a child network with the same topology and functions

	for (size_t i = 0; i < parent1.m_Model.size(); ++i) {
		child.m_Model[i] = Layer::Crossover(parent1.m_Model[i], parent2.m_Model[i]);
	}

	return child;
}

NeuralNetwork NeuralNetwork::MutateNetwork(const NeuralNetwork& network, float mutationRate) {
    NeuralNetwork mutatedNetwork(network);

    for (auto& layer : mutatedNetwork.m_Model) {
        layer = Layer::MutateLayer(layer, mutationRate);
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

