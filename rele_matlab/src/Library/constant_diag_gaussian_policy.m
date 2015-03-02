%%% Gaussian with constant mean and diagonal covariance.
%%% Both mean and variance are learned.
classdef constant_diag_gaussian_policy
    
    properties(GetAccess = 'public', SetAccess = 'private')
        % These properties are public by default
        dim;
        dim_variance_params;
    end
    
    properties(GetAccess = 'public', SetAccess = 'public')
        theta
    end
    
    methods
        
        function obj = constant_diag_gaussian_policy(dim, init_mean, init_sigma)
            % Class constructor
            obj.theta = [init_mean; init_sigma];
            obj.dim = dim;
            obj.dim_variance_params = length(init_sigma);
        end
        
        function param = drawAction(obj)
            mu = vec2mat(obj.theta(1:obj.dim),obj.dim);
            sigma = diag(obj.theta(obj.dim+1:end));
            param = mvnrnd(mu,sigma^2)';
        end
        
        % differential entropy, can be negative
        function H = entropy(obj, state)
            sigma = diag(obj.theta(obj.dim+1:end));
            H = 0.5*log( (2*pi*exp(1))^obj.dim * det(sigma^2) );
        end
        
        function probability = evaluate(obj, theta)
            assert(size(theta,2) == 1);
            mu = vec2mat(obj.theta(1:obj.dim),obj.dim);
            sigma = diag(obj.theta(obj.dim+1:end));
            probability = mvnpdf(theta, mu, sigma^2);
        end
        
        function dlogpdt = dlogPidtheta(obj, theta)
            if (nargin == 1)
                % Return the dimension of the gradient
                dlogpdt = length(obj.theta);
                return
            end
            assert(size(theta,2) == 1);
            mu = vec2mat(obj.theta(1:obj.dim),obj.dim);
            sigma = obj.theta(obj.dim+1:end);
            
            dlogpdt_mu = sigma.^-2 .* (theta - mu);
            dlogpdt_sigma = -sigma.^-1 + (theta - mu).^2 ./ sigma.^3;
            
            dlogpdt = [dlogpdt_mu; dlogpdt_sigma];
        end
        
        function obj = update(obj, direction)
            obj.theta = obj.theta + direction;
        end
        
        function obj = makeDeterministic(obj)
            obj.theta(obj.dim+1:end) = 1e-8;
        end
        
        function [mu, sigma] = getMuSigma(obj)
            mu = vec2mat(obj.theta(1:obj.dim),obj.dim);
            sigma = diag(obj.theta(obj.dim+1:end));
        end
        
        function obj = weightedMLUpdate(obj, d, Theta)
            assert(size(Theta,2) == size(d,1));
            assert(size(d,2) == 1);
            mu = Theta * d / sum(d);
            sigma = zeros(obj.dim,1);
            for k = 1 : size(Theta,2)
                sigma = sigma + (d(k) * (Theta(:,k) - mu).^2);
            end
            Z = (sum(d)^2 - sum(d.^2)) / sum(d);
            sigma = sigma / Z;
            sigma = sqrt(sigma);
            obj.theta = [mu; sigma(:)];
        end
                
    end
    
end
