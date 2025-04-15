clc;
clear;

% model pv values (estimations)
Voc = 18.5;      % Voc
Isc = 0.52;     % Isc
Vmpp = 15.9;     % voltage at MPP (LOOSELY RELATED TO THE PANEL OBSERVATIONS FROM DAVIDS LABS)
Impp = 0.47;    % current at MPP
MPP_duty = 25; % duty cycle where MPP should occur (FIXED)

V = linspace(0, Voc, 1000);

% IV curve to map to
I = zeros(size(V));
for i = 1:length(V)
    if V(i) < Vmpp
        %current drops from Isc to Impp as voltage increases to Vmpp
        I(i) = Isc - (Isc - Impp) / Vmpp * V(i);
    else
        %current drops from Impp to 0 (for simplicity it is a linar aprox)
        I(i) = Impp * (1 - (V(i) - Vmpp) / (Voc - Vmpp));
    end
end
I(I < 0) = 0; %current clamp
P = V .* I; %calculate power at each point of V

% setup for MPPT
duty = 100.0;          % start duty (arbitrary number can mess arround)
step = 0.5;           %step size used (same as code)
prev_P = 0;           %storage of previous values
prev_V = 0;

%maps the boost converter effects (pannel voltage drps while duty cycle increases) by
%scaling factor K (also ensures Vin=Vmpp when duty = MPP_duty
k = (Voc - Vmpp) / MPP_duty;
duty_to_voltage = @(duty) Voc - k * duty;

% create figure of two plots
figure('Position', [100, 100, 1000, 500]);
%plot for pv curve and tracker marker
subplot(1, 2, 1);
plot(V, P, 'b', 'LineWidth', 2); hold on;
tracker1 = plot(0, 0, 'ro', 'MarkerSize', 8, 'MarkerFaceColor', 'r');
xlabel('Voltage (V)');
ylabel('Power (W)');
title('MPPT Tracking on PV Curve');
grid on;
xlim([0 Voc]);
ylim([0 max(P)*1.1]);
%plot for the duty cycle changes over time
subplot(1, 2, 2);
duty_line = animatedline('Color', 'magenta', 'LineWidth', 2);
xlabel('Iteration');
ylabel('Duty Cycle (%)');
title('Duty Cycle Evolution');
grid on;
ylim([0 100]);
xlim([0 100]);

% simulation of my algoritham (or 100 itterations)
for itterations = 1:100
    % estimate Vin from duty using model above 
    Vin = duty_to_voltage(duty);
    Iin = interp1(V, I, Vin, 'linear', 0);
    Pin = Vin * Iin;

    delta_P = Pin - prev_P;
    delta_V = Vin - prev_V;

    % P&O logic same as my final P&O algoritham
    if abs(delta_P) < 0.01
        break;
    else
        if delta_P > 0
            if delta_V > 0
                direction = -1;
            else
                direction = 1;
            end
            step_mult = 2.0;
        else
            if delta_V > 0
                direction = 1;
            else
                direction = -1;
            end
            step_mult = 3.0;
        end
        duty = duty + direction * step * step_mult;
    end


    % update plots 
    subplot(1, 2, 1);
    set(tracker1, 'XData', Vin, 'YData', Pin);
    
    subplot(1, 2, 2);
    addpoints(duty_line, itterations, duty);

    drawnow;
    pause(0.1);

    % update previous values
    prev_P = Pin;
    prev_V = Vin;
end
