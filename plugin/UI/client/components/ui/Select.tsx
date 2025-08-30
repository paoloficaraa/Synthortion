import React from "react";

interface SelectProps {
  value: number;
  onChange: (value: number) => void;
  options: { value: number; label: string }[];
  className?: string;
}

const Select: React.FC<SelectProps> = ({
  value,
  onChange,
  options,
  className = "",
}) => {
  return (
    <div className={`relative ${className}`}>
      <select
        value={value}
        onChange={(e) => onChange(Number(e.target.value))}
        className="w-full bg-gray-700 border border-gray-600 rounded-md px-3 py-2 text-white text-sm font-medium 
                   focus:outline-none focus:ring-2 focus:ring-orange-500 focus:border-orange-500
                   appearance-none cursor-pointer hover:bg-gray-600 transition-colors"
        style={{
          backgroundImage: `url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' fill='none' viewBox='0 0 20 20'%3e%3cpath stroke='%236B7280' stroke-linecap='round' stroke-linejoin='round' stroke-width='1.5' d='M6 8l4 4 4-4'/%3e%3c/svg%3e")`,
          backgroundPosition: "right 0.5rem center",
          backgroundRepeat: "no-repeat",
          backgroundSize: "1.5em 1.5em",
          paddingRight: "2.5rem",
        }}
      >
        {options.map((option) => (
          <option
            key={option.value}
            value={option.value}
            className="bg-gray-700"
          >
            {option.label}
          </option>
        ))}
      </select>
    </div>
  );
};

export default Select;
