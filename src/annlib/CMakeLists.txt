if (ANN_CUDA_ACTIVE)
    include_directories(cuda)
    add_subdirectory(cuda)
endif (ANN_CUDA_ACTIVE)

add_library(annlib
        _calc_macros.h
        activation_function.h activation_function.cpp
        convolution_layer.cpp convolution_layer.h
        gradient_based_optimizer.h gradient_based_optimizer.cpp
        mini_batch_builder.h mini_batch_builder.cpp
        network_layer.h network_layer.cpp
        output_layer.h output_layer.cpp
        sgd_trainer.h sgd_trainer.cpp
        training_buffer.h training_buffer.cpp
        training_data.h training_data.cpp
        weight_norm_penalty.h weight_norm_penalty.cpp)

add_dependencies(annlib linalg)
if (ANN_CUDA_ACTIVE)
    add_dependencies(annlib annlib_cudaops)
endif (ANN_CUDA_ACTIVE)

install(TARGETS annlib DESTINATION lib)
install(FILES
        _calc_macros.h
        activation_function.h
        convolution_layer.h
        gradient_based_optimizer.h
        mini_batch_builder.h
        network_layer.h
        output_layer.h
        sgd_trainer.h
        training_buffer.h
        training_data.h
        weight_norm_penalty.h
        DESTINATION include)