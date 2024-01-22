#pragma once

#include <map>
#include <string>
#include "matrix.hpp"

namespace ActivationFunction{
	using Ptr = float (*)(float);

	ActivationFunction::Ptr Find(const std::string& name);
}

class Layer {
	Matrix<float> m_Weights;
	Matrix<float> m_Biases;
	
	ActivationFunction::Ptr m_Function;
	std::string m_FunctionName;
public:
	Layer(Layer &&layer) = default;
	Layer(const Layer &layer) = default;

	Layer &operator=(Layer &&layer) = default;
	Layer &operator=(const Layer &layer) = default;

	Layer(Matrix<float> weights, Matrix<float> biases, const std::string &function_name);

	Layer(int input_size, int output_size, ActivationFunction::Ptr function, const std::string &function_name);

	Matrix<float> Do(const Matrix<float>& input)const;

	const Matrix<float>& Weights()const;

	const Matrix<float>& Biases()const;

	const std::string& FunctionName()const;

	bool IsComplete()const;

	static void Mix(Matrix<float>& output, const Matrix<float>& left, const Matrix<float>& right, float rate = 0.5f);

	static Layer Crossover(const Layer& parent1, const Layer& parent2);

	static Layer MutateLayer(const Layer& layer, float mutationRate);
};

class NeuralNetwork {
	std::vector<Layer> m_Model;
	std::vector<int> m_Topology;
	std::vector<std::string> m_Functions;
public:
	NeuralNetwork(NeuralNetwork &&) = default;
	NeuralNetwork(const NeuralNetwork &) = default;

	NeuralNetwork &operator=(NeuralNetwork &&) = default;
	NeuralNetwork &operator=(const NeuralNetwork &) = default;

	NeuralNetwork(std::vector<Layer> model, std::vector<int> topology, std::vector<std::string> functions);

	NeuralNetwork(std::vector<int> topology, std::vector<std::string> functions);

	Matrix<float> Do(Matrix<float> Input);

	static NeuralNetwork Crossover(const NeuralNetwork& parent1, const NeuralNetwork& parent2);

	static NeuralNetwork MutateNetwork(const NeuralNetwork& network, float mutationRate);

	const std::vector<Layer>& Layers()const;

	const std::vector<int>& Topology()const;

	const std::vector<std::string>& Functions()const;
};


namespace Serialization {
	
	inline void ToStream(const Layer& layer, std::ostream& stream) {
		ToStream(layer.Weights(), stream);
		ToStream(layer.Biases(), stream);
		ToStream(layer.FunctionName(), stream);
	}

	inline Layer FromStreamImpl(std::istream& stream, Layer*) {
		auto w = FromStream<Matrix<float>>(stream);
		auto b = FromStream<Matrix<float>>(stream);
		auto s = FromStream<std::string>(stream);
		return Layer(std::move(w), std::move(b), std::move(s));
	}


	inline void ToStream(const NeuralNetwork& nn, std::ostream& stream) {
		ToStream(nn.Layers(), stream);
		ToStream(nn.Topology(), stream);
		ToStream(nn.Functions(), stream);
	}

	inline NeuralNetwork FromStreamImpl(std::istream& stream, NeuralNetwork *) {
		auto l = 
			FromStream<std::vector<Layer>>(stream);
		auto t = 
			FromStream<std::vector<int>>(stream);
		auto f = 
			FromStream<std::vector<std::string>>(stream);
		return NeuralNetwork(std::move(l), std::move(t), std::move(f));
	}
}
